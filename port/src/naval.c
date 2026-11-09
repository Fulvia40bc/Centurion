/* The naval battle, OVL0 section 5 (file 0x8550-0xB3F0, loaded at 0823:B0E0), Claude 2026-11-09.
 * Translated literally from disasm/OVL_NAVAL.asm (addresses in the comments), after docs/NAVAL_LOGIC_NOTES.md,
 * NAVAL_SCREEN_NOTES.md, NAVAL_DATA_NOTES.md and NAVAL_INDEX.md. The drawing routines and the EXE drawing helpers are in
 * naval_gfx.c; naval_int.h names the DS state block (side 0 = the enemy flagship, 1 = the Roman flagship).
 *
 *   B0E0  fleet enters a sea zone: fights every enemy fleet 3..5 there        B12A  the battle (entry)
 *   BD8A  set-up: C274 the flagship menu, BF50 loading, C105 the intro, BE0F   B59F  the result message
 *   the duel, one pass: C4F8/C558 distance, C641 bearing, C366 the player, B6A0 the enemy AI, C3D3 turn, C422 oars
 *   (and the rowing drum), B7FA move, D242 screen edges, CF7F/CFDD hull rects and the ship in front, CD96/CE7E ramming,
 *   CB11/CC4D fire pots, C92B/C9A3/D11B arrows, a background round of the fleet battle (DA50) every 2..10 s, the frame
 *   every 3 ticks (naval_gfx.c), BB7E/BD41 boarding, the re-engage question; then D3F2/D60C the sinking, the fleet
 *   battle DA50/DD2E/DE51 until one fleet is gone, the write-back and B59F.
 *
 * TIME (the port's pacing, NAVAL_SCREEN_NOTES 3.3). The original's duel loop is not throttled: everything is gated by
 * the 72.8 Hz timer (DS:42/44 and the 16 slots DS:46..; EXE 00D7/00F8/00FF/0183/01AE/011F, translated literally here)
 * except the enemy AI B6A0, whose dice (rng(3) or rng(2) every pass) and the collision test run on every pass, so the
 * 1990 dice depended on the CPU speed. The port runs ONE PASS PER TICK: after each pass the tick counter DS:42/44 goes
 * up by one (nv_tick); the busy waits (the sinking frames 14 ticks, the boarding frames 9, the boarding pause 36, the
 * intro's polls) tick the counter until they are over. In the window each tick waits for real time (72.8 Hz, by
 * platform_ticks_ms); headless (--script) no time passes, so a run is fully determined by the dice and the script's
 * input per tick ("hold KEY N", "wait N", "key K" lines, see screen.c). Frames are drawn every 3 ticks as in the original
 * (burning, the corvus and the speed word count frames). The input: held arrows / keypad = DS:A2 bits (screen_held_dirs),
 * keys by 03AA once per pass: 'r' retreat, Enter or '+' the corvus, Space or Insert a fire pot. Port addition: when the
 * script runs out or the window is closed during the duel, the duel ends as with 'r' (the original has no way out).
 * Without a screen (--end-turn) the battle cannot be fought: B12A then says so and returns 0 (cancelled), as before.
 *
 * ERIK'S REPORT ("I always lost the battle even after sending the enemy's witless commander to Neptune's depths").
 * Decoded (D): the result is set ONLY from the ship counts after the fleet battle (b506/b513/b536): Roman fleet empty
 * = lost, enemy fleet empty = won, both left = retreat; the sums are taken over the right fleets, and the texts are not
 * inverted: "Alas, your ship duel was with the witless commander of the enemy fleet, and his more competent mates have
 * swept the seas clean of Roman vessels" (DS:9220, b604) is the LOST text for "your flagship afloat" and was written for
 * exactly this case. The duel decides nothing by itself: the fleet battle DA50 is a dice race in which every hit sinks a
 * whole ship, it already runs every 2..10 s DURING the duel at the full odds (b2f4..b321, the SHIPS numbers in the panel
 * fall), and after the duel it runs to the end (b4ac). Sinking the enemy flagship only (1) keeps it out of its fleet,
 * (2) lowers each enemy ship's hit chance from 99C2 to 99CE (trireme 4% -> 2%, quinquereme 6% -> 3%, galleon 9% -> 5%)
 * and (3) its 4 flagship rolls to 1%. The enemy fleets are big (Carthage 50 quinqueremes, Aegyptus 100 galleons,
 * Macedonia 30 quinqueremes, plus 5000..8000 soldiers who also fire), so a Roman fleet below about 60..80% of the enemy's
 * loses even after the sinking. That is the design, not a bug, and the port keeps it (the knobs, if Erik wants the duel
 * to weigh more, would be 99CE or stopping the background rounds during the duel: not done).
 * Three 1990 bugs on the way, fixed by default (--original keeps them):
 *   9.1  the burning wreck: D3F2 draws the sunk flagship with B8E3, whose flames (CD4B) took its hull from 0 to -1, and
 *        b49e/b4a5 test == 0: the fleet battle was skipped, and a duel won by fire ended "You retreat to safety with your
 *        remaining ships" - no victory; a burning survivor could also sink after the duel. Fix: the flames burn the
 *        hulls only while the duel runs (nv_burn_on, naval_gfx.c CD4B).
 *   13.1 ram damage wraps: CD96 computes rng(16) + factor * oar level - 8 in a byte: below 0 a full ram healed the
 *        victim's hull but killed up to 255 men, and a scrape halved 0xF8..0xFF into 124..127 hull damage - a flagship
 *        sunk outright by a slow scrape (19..50% of rams at low oar levels). Fix: the damage is at least 0 (the same
 *        dice are rolled: rng(0) still rolls).
 *   13.3 a hull below 0 is not "sunk": the duel loop and the outcome test == 0, only the panel (every 3 ticks) clamps.
 *        Fix: <= 0.
 * Other 1990 bugs fixed by default: 13.5 the casualty split of DD2E / DE51 in 16 bits (garbled cavalry / infantry or
 * army part losses: 32-bit products), 13.6 negative fixed-point fractions (ships, pots and arrows moved other amounts
 * west / north than east / south), 13.7 an empty Roman fleet at the flagship menu (an uninitialised local in 1990: the
 * port cancels the battle; --original takes the trireme, G), the second shark's start (naval_gfx.c D60C), the
 * cycled water colours never restored (C18C has no caller; naval_gfx.c), the MEN line printed in the big box font after
 * "No" to re-engage (b3db sets A05C, nothing puts A3B4 back before b479's C71B; NAVAL_SCREEN_NOTES 9.2) and a battle
 * started by B0E0 / 4ADA with a Roman fleet already gone (FLEET_NOTES 9.3; fleet.c), and 13.2: BB7E gave the boarding
 * +10 courage to the side that was boarded (bits 95D2[s] = 1 << s, but bit 2 raised C36E[0]) - a bit-to-index swap, not
 * a defender's advantage: the corvus is Rome's own boarding weapon and the text cheers "Your sailors cheer lustily"
 * (ruled 10 Nov; Erik deferred to Claude). Also kept: a sunk flagship's 1% rolls in DA50 (13.4), the
 * fled admiral's ship rejoining its fleet (b45f before b493), the enemy's fixed Senator row 99C2, D242's double push.
 * Easter eggs kept: the witless commander, "In spite of their admiral's bath" (won after YOUR flagship sank: bit 0 is
 * the Roman, so "their" is the fleet's own admiral), "Neptune's locker without a sword", "together forever in a deadly
 * embrace", the boarding texts, RAMMING SPEED with the rowing drum (cn9a..cn9d) speeding up, the sharks that circle each
 * wreck and dive one time in ten, the Roman "Galleon".
 */
#include <stdio.h>
#include <string.h>
#include "state.h"
#include "screen.h"
#include "platform.h"
#include "naval.h"
#include "naval_int.h"

void music_load_5D73(uint8_t m);          /* map.c (the DS part of 5D73) */

int nv_burn_on;
static int8_t g_col[816];                 /* COLLIDE.LZW: the buffer the pointers C3B0..C3B8 point into (offsets here) */

static const char *dstr(uint16_t a) { return (const char *)&g_ds[a]; }
static size_t ds_len(uint16_t a) { size_t n = 0; while ((size_t)a + n < 0x10000 && g_ds[a + n]) n++; return n; }
static void ds_strcat(uint16_t d, uint16_t s) {                   /* 9DF2 */
    size_t at = (size_t)d + ds_len(d), n = ds_len(s);
    if (at + n + 1 > 0x10000) n = at < 0xFFFF ? 0xFFFF - at : 0;
    memmove(&g_ds[at], &g_ds[s], n);
    if (at + n < 0x10000) g_ds[at + n] = 0;
}
static void ds_strcpy(uint16_t d, uint16_t s) { g_ds[d] = 0; ds_strcat(d, s); }   /* 9E32 */

uint16_t nv_rng(uint16_t n) { return rng_roll(n); }
static int sub_050F(int v, int step, int lim) { int r = (int16_t)(v - step); return r < lim ? lim : r; }   /* max(v - step, lim) */
/* the idiom cwd / xor / sub / sar 8 / xor / sub: v / 256 toward 0 */
static int16_t div256(int16_t v) { int a = v < 0 ? -v : v; a >>= 8; return (int16_t)(v < 0 ? -a : a); }
/* the fraction kept after a step: 1990 `frac = v & 0FFh; if (v < 0) frac = -frac` (13.6); fixed: -(|v| & 0FFh) */
static uint16_t frac(int16_t v) {
    if (g_original_bugs) { uint16_t f = (uint16_t)(v & 0xFF); return v < 0 ? (uint16_t)-f : f; }
    return v < 0 ? (uint16_t)-((-v) & 0xFF) : (uint16_t)(v & 0xFF);
}
static int16_t sin_C4D8(int h) { return (int16_t)SW(0x987A + 2 * h); }
static int16_t cos_C4E8(int h) { return (int16_t)SW(0x989A + 2 * h); }
static int col(int table, int i) { return g_col[(W(0xC3B0 + 2 * table) + i) % 816]; }   /* signed byte of table C3B0..C3B8 */
static int sunk(int s) { return g_original_bugs ? SB(HULL(s)) == 0 : SB(HULL(s)) <= 0; }   /* 13.3 */

/* ---------------------------------------------------------------- time (EXE 00D7..01AE, 011F) and the port's clock */
static uint32_t now32(void) { return (uint32_t)W(0x42) | (uint32_t)W(0x44) << 16; }
void nv_t_start_00D7(int n) { n &= 15; setW((uint16_t)(0x46 + 4 * n), W(0x42)); setW((uint16_t)(0x48 + 4 * n), W(0x44)); }
void nv_t_reset_00F8(void) { nv_t_start_00D7(0); }
void nv_t_all_00FF(void) { for (int n = 0; n < 16; n++) nv_t_start_00D7(n); }
/* 0183(n): ticks since slot n; FFFF when the high words differ */
uint16_t nv_t_since_0183(int n) {
    n &= 15;
    if (W(0x44) != W(0x48 + 4 * n)) return 0xFFFF;
    return (uint16_t)(W(0x42) - W(0x46 + 4 * n));
}
uint16_t nv_t_since0_01AE(void) { return nv_t_since_0183(0); }
/* 011F(n): busy-wait until slot 0 + n <= now (slot 0 not restarted) */
void nv_wait_011F(uint16_t n) {
    uint32_t t = ((uint32_t)W(0x46) | (uint32_t)W(0x48) << 16) + n;
    while (t > now32()) nv_tick();
}
static uint32_t g_rt_base, g_rt_n;
static int g_rt_on;
void nv_tick(void) {
    uint32_t t = now32() + 1;
    setW(0x42, (uint16_t)t); setW(0x44, (uint16_t)(t >> 16));
    screen_tick();
    if (screen_headless()) return;
    uint32_t now = platform_ticks_ms();
    if (!g_rt_on) { g_rt_on = 1; g_rt_base = now; g_rt_n = 0; }
    g_rt_n++;
    uint32_t due = g_rt_base + (uint32_t)((uint64_t)g_rt_n * 100000u / 7283u);
    if ((int32_t)(now - due) > 100) { g_rt_base = now; g_rt_n = 0; }   /* far behind (a box waited for a click): afresh */
    else if ((int32_t)(due - now) > 0) platform_sleep_ms(due - now);
}

/* ---------------------------------------------------------------- music (the DS part, as map.c / nego.c) */
static void music_stop_5D20(void) { B(0x3EEE) = 0xFF; }
static void music_unload_5E1A(uint8_t n) {
    for (int i = 0; i < 4; i++) {
        if (B(0xA6B2 + 4 * i) == 0 || B(0xA6B3 + 4 * i) != n) continue;
        setW((uint16_t)(0xA6B0 + 4 * i), 0); B(0xA6B2 + 4 * i) = 0; B(0xA6B3 + 4 * i) = 0xFF; B(0x3EEE) = 0xFF;
        break;
    }
}

/* ---------------------------------------------------------------- geometry */
/* C558(&iter, N): integer square root by Newton steps in longs, the error |N - t*t| kept in 16 bits, at most 20 steps,
   then x + 1 or x - 1 when that error is smaller */
static uint16_t isqrt_C558(uint16_t *iter, uint32_t N) {
    uint16_t nl = (uint16_t)N;
    uint32_t x = 0x8C;                                               /* [bp-0Ch] */
    uint32_t prev = (uint32_t)(int32_t)(int16_t)(nl - 0x4C90);
    prev = (uint32_t)(int32_t)(int16_t)((int16_t)prev < 0 ? -(int16_t)prev : (int16_t)prev);
    uint16_t hi = 0;                                                 /* [bp-6] */
    for (;;) {
        uint32_t t = (N / x + x) >> 1;
        hi = (uint16_t)(t >> 16);
        int16_t d = (int16_t)(nl - (uint16_t)((uint16_t)t * (uint16_t)t));
        uint32_t e = (uint32_t)(int32_t)(int16_t)(d < 0 ? -d : d);
        if (e == prev) break;
        x = t; prev = e;
        if (++*iter == 20) break;
    }
    uint32_t c = x + 1;
    hi = (uint16_t)(c >> 16);
    int16_t d = (int16_t)(nl - (uint16_t)((uint16_t)c * (uint16_t)c));
    uint32_t e = (uint32_t)(int32_t)(int16_t)(d < 0 ? -d : d);
    if (e < prev) x = (uint32_t)(uint16_t)c | (uint32_t)hi << 16;
    else {
        c = x - 1;
        hi = (uint16_t)(c >> 16);
        d = (int16_t)(nl - (uint16_t)((uint16_t)c * (uint16_t)c));
        e = (uint32_t)(int32_t)(int16_t)(d < 0 ? -d : d);
        if (e < prev) x = (uint32_t)(uint16_t)c | (uint32_t)hi << 16;
    }
    return (uint16_t)x;
}
/* C4F8(x0, y0, x1, y1): the distance between the ships */
static uint16_t dist_C4F8(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    uint16_t iter = 0;
    int16_t dx = (int16_t)(x1 - x0), dy = (int16_t)(y1 - y0);
    dx = dx < 0 ? -dx : dx; dy = dy < 0 ? -dy : dy;
    uint32_t n = (uint32_t)((int32_t)dx * dx) + (uint32_t)((int32_t)dy * dy);
    return n ? isqrt_C558(&iter, n) : 0;
}
/* C641(&enemy, &Roman): the heading h whose step of d (the distance, or a quarter of it above 100) from the Roman
   comes closest to the enemy = the bearing from the Roman to the enemy */
static uint8_t bearing_C641(void) {
    uint32_t best = 0x249F0;
    uint8_t bh = 0;                                                  /* [bp-0Dh] (always set in practice) */
    uint16_t d = W(DIST) > 0x64 ? (uint16_t)(W(DIST) >> 2) : W(DIST);
    for (int h = 0; h < 16; h++) {
        int16_t sx = div256((int16_t)(sin_C4D8(h) * (int16_t)d));
        int16_t sy = div256((int16_t)(cos_C4E8(h) * (int16_t)d));
        int16_t ex = (int16_t)(SW(PX(0)) - SW(PX(1)) - sx), ey = (int16_t)(SW(PY(0)) - SW(PY(1)) - sy);
        ex = ex < 0 ? -ex : ex; ey = ey < 0 ? -ey : ey;
        uint32_t s = (uint32_t)((int32_t)ex * ex) + (uint32_t)((int32_t)ey * ey);
        if (s < best) { bh = (uint8_t)h; best = s; }
    }
    return bh;
}
/* 05B0 / 05ED: point in circle, circles touch (squares in 16 bits, unsigned compare) */
static int in_circle(int x, int y, int cx, int cy, int r) {
    uint16_t a = (uint16_t)(cx - x), b = (uint16_t)(cy - y);
    a = (int16_t)a < 0 ? (uint16_t)-a : a; b = (int16_t)b < 0 ? (uint16_t)-b : b;
    return (uint16_t)((uint16_t)(a * a) + (uint16_t)(b * b)) <= (uint16_t)(r * r);
}
static int circles_touch(int x0, int y0, int r0, int x1, int y1, int r1) { return in_circle(x0, y0, x1, y1, r0 + r1); }
static void rect_of(int s, int16_t *r) { for (int i = 0; i < 4; i++) r[i] = (int16_t)SW(RECT(s) + 2 * i); }
static int col_index(int s) { return B(0x98E2 + SB(HEAD(s))) + B(0x98F2 + B(TYPE(s))); }

/* D070(x, y, s): the point in ship s's rect and in one of its 5 circles */
int nv_d070(int x, int y, int s) {
    int16_t r[4];
    rect_of(s, r);
    if (!nv_inrect_8238(x, y, r)) return 0;
    int px = SW(PX(s)), py = SW(PY(s)), i = col_index(s);
    for (int k = 0; k < 5; k++, i++)
        if (in_circle(x, y, col(0, i) + px, col(1, i) + py, col(2, i))) return -1;
    return 0;
}
/* CF7F(s): the ship's sprite rect (hull frame = heading) */
static void rect_CF7F(int s) {
    int w, xh, yh, rows;
    nv_desc(W(SHEET(s)), SB(HEAD(s)), &w, &xh, &yh, &rows);
    int16_t x0 = (int16_t)(SW(PX(s)) - xh), y0 = (int16_t)(SW(PY(s)) - yh);
    setW(RECT(s), (uint16_t)x0); setW(RECT(s) + 4, (uint16_t)(w + x0));
    setW(RECT(s) + 2, (uint16_t)y0); setW(RECT(s) + 6, (uint16_t)(rows + y0));
}
/* CFDD: if C384's bow is not in the other ship but the other's bow is in C384, the other becomes C384 */
static void front_CFDD(void) {
    int R = W(RAMMER), o = R ^ 1;
    int i = 16 * B(TYPE(R)) + SB(HEAD(R));
    if (nv_d070(col(3, i) + SW(PX(R)), col(4, i) + SW(PY(R)), o)) return;
    i = 16 * B(TYPE(o)) + SB(HEAD(o));
    if (nv_d070(col(3, i) + SW(PX(o)), col(4, i) + SW(PY(o)), R)) setW(RAMMER, (uint16_t)o);
}
/* CE7E: the rects overlap and a Roman circle touches an enemy circle */
static int touch_CE7E(void) {
    int16_t a[4], b[4], out[4];
    rect_of(0, a); rect_of(1, b);
    if (!nv_rect_817C(out, b, a)) return 0;
    int rx = SW(PX(1)), ry = SW(PY(1)), ex = SW(PX(0)), ey = SW(PY(0));
    int ri = B(0x98E2 + SB(HEAD(1))) + B(0x98F2 + B(TYPE(1)));
    int ei = B(0x98E2 + SB(HEAD(0))) + B(0x98F2 + B(TYPE(0)));
    for (int k = 0; k < 5; k++, ri++)
        for (int j = 0; j < 5; j++)
            if (circles_touch(col(0, ri) + rx, col(1, ri) + ry, col(2, ri), col(0, ei + j) + ex, col(1, ei + j) + ey, col(2, ei + j)))
                return -1;
    return 0;
}
/* D11B(s): the arrow volley of s ^ 1 hits ship s (its frame rect meets s's rect, its circle 98F6/9906/9916[dir] touches
   one of s's circles) */
static int arrows_D11B(int s) {
    int o = s ^ 1, dir = SB(ADIR(o));
    int w, xh, yh, rows;
    int16_t a[4], r[4], out[4];
    nv_desc(W(H_BOMBS), dir + 0x20, &w, &xh, &yh, &rows);           /* 14 * dir + [C3AE] + 0x1C0 */
    a[0] = (int16_t)(SW(ARRX(o)) - xh); a[2] = (int16_t)(w + a[0]);
    a[1] = (int16_t)(SW(ARRY(o)) - yh); a[3] = (int16_t)(rows + a[1]);
    rect_of(s, r);
    if (!nv_rect_817C(out, r, a)) return 0;
    int vx = SB(0x98F6 + dir) + SW(ARRX(o)), vy = SB(0x9906 + dir) + SW(ARRY(o)), vr = B(0x9916 + dir);
    int px = SW(PX(s)), py = SW(PY(s)), i = col_index(s);
    for (int k = 0; k < 5; k++, i++)
        if (circles_touch(col(0, i) + px, col(1, i) + py, col(2, i), vx, vy, vr)) return -1;
    return 0;
}

/* ---------------------------------------------------------------- the player and the enemy */
/* C846(s): a fire pot straight ahead, when none flies, no arrows fly and 145 ticks passed; range = distance minus a lead
   (98D0[|own - other heading|]; halved against a slow target), at least 90 */
static void pot_C846(int s) {
    int n = 8 * s + 5;
    if (B(POT(s)) || B(ARROW(s)) || !(nv_t_since_0183(n) > 0x90)) return;
    nv_t_start_00D7(n);
    B(POT(s)) = 0xFF;
    B(PDIR(s)) = B(HEAD(s));
    setW(POTX(s), W(PX(s))); setW(POTY(s), W(PY(s)));
    setW(POTFX(s), W(0x56FE)); setW(POTFY(s), W(0x5700));
    B(POTANIM(s)) = 0;
    setW(PRANGE(s), W(DIST));
    int dh = SB(HEAD(s)) - SB(HEAD(s ^ 1));
    int k = SB(0x98D0 + (dh < 0 ? -dh : dh));                       /* [bp-4] */
    int16_t lead = k ? (int16_t)((int16_t)W(DIST) / k) : 0;         /* [bp-2] */
    if (SB(GEAR(s ^ 1)) < 4) lead = (int16_t)(lead / 2);
    setW(PRANGE(s), (uint16_t)sub_050F((int16_t)W(DIST), lead, 0x5A));
}
/* C366: the Roman's controls: held directions (DS:A2) set the oar / turn requests; one key: 'r' = 4 (retreat),
   Enter or '+' = the corvus for 20 frames, Space or Insert = a fire pot */
static uint16_t input_C366(void) {
    uint16_t si = 0;
    B(0xA2) = (uint8_t)screen_held_dirs();                           /* the INT 9 hook's bits (port: window / script) */
    if (B(0xA2)) {
        if (B(0xA2) & 8) B(OARREQ(1)) = 1;
        if (B(0xA2) & 4) B(OARREQ(1)) = 0xFF;
        if (B(0xA2) & 2) B(TURNREQ(1)) = 0xFF;
        if (B(0xA2) & 1) B(TURNREQ(1)) = 1;
    }
    switch (key_read() & 0xFF) {                                     /* 03AA */
    case 0x72: si = 4; break;                                        /* 'r' (lower case only) */
    case 0x0D: case 0x2B: B(HOOKS(1)) = 0x14; break;
    case 0x10: case 0x20: pot_C846(1); break;
    }
    if (!si && screen_quit()) si = 4;                                /* port: no more input -> retreat */
    return si;
}
/* B6A0: the enemy AI (side 0) */
static void ai_B6A0(void) {
    int weak = 0, strong = 0;                                        /* [bp-4], [bp-6] */
    if ((int8_t)(B(0x99C2 + B(TYPE(0))) << 2) >= SB(HULL(1))) weak = -1;
    int16_t cx = (int16_t)(SB(COUR(0)) * (int16_t)W(CREW(0)));
    int16_t ax = (int16_t)(SB(COUR(1)) * (int16_t)W(CREW(1)));
    if (ax > cx) strong = -1;
    uint8_t h = B(HEAD(0));                                          /* [bp-1] */
    if (B(TURNREQ(0)) == 0) {
        uint8_t rel = (uint8_t)((h - B(BEAR(0))) & 0x0F);            /* [bp-2] */
        if (rel == 8) rel = (uint8_t)nv_rng(16);
        int8_t t = SB(0x93A6 + (int8_t)rel);                         /* [bp-7] */
        if (rel == 0) pot_C846(0);
        if (t == 0) {
            int hi = (int8_t)h;
            int16_t si = (int16_t)(SB(0x93C6 + hi) * (int16_t)(SW(PY(0)) - SW(PY(1))));
            int16_t di = (int16_t)(SB(0x93B6 + hi) * (int16_t)(SW(PX(0)) - SW(PX(1))));
            if (di) si = (int16_t)(si < 0 ? -si : si);
            if (si) di = (int16_t)(di < 0 ? -di : di);
            if (si < di) t = SB(0x93D6 + hi);
            else if (si > di) t = (int8_t)-SB(0x93D6 + hi);
        }
        if (weak || strong || W(DIST) > 0x5F) B(TURNREQ(0)) = (uint8_t)t;
        if (B(POT(1))) B(TURNREQ(0)) = (uint8_t)(nv_rng(3) - 1);
    }
    int8_t target = weak ? 3 : 2;
    if (target >= SB(OARS(0))) B(OARREQ(0)) = (uint8_t)nv_rng(2);
    else B(OARREQ(0)) = (uint8_t)(nv_rng(3) - 1);
    if (strong && W(DIST) < 0x55) nv_hooks_BA7A(0);                  /* dead: C3AA[0] is never set */
}
/* C3D3(s): when 9828[type] ticks passed, a pending turn request turns a moving ship one step */
static void turn_C3D3(int s) {
    int n = 8 * s + 3;
    if (!(B(0x9828 + B(TYPE(s))) < nv_t_since_0183(n))) return;
    if (B(GEAR(s)) != 0 && B(TURNREQ(s)) != 0) {
        B(HEAD(s)) = (uint8_t)((B(HEAD(s)) + B(TURNREQ(s))) & 0x0F);
        nv_t_start_00D7(n);
    }
    B(TURNREQ(s)) = 0;
}
/* C422(s): when 982C[type] ticks passed, the oar level follows the request (to 0 only with the oars at rest); the speed
   index from 9818; the Roman's speed word for 10 frames and the rowing drum (cn9a..cn9d, silence when stopped) */
static void oars_C422(int s) {
    int n = 8 * s + 4;
    if (!(B(0x982C + B(TYPE(s))) < nv_t_since_0183(n))) return;
    if (B(OARREQ(s)) != 0) {
        B(OARS(s)) = (uint8_t)(B(OARS(s)) + B(OARREQ(s)));
        if (SB(OARS(s)) > 4) { B(OARS(s)) = 4; B(OARREQ(s)) = 0; }
        else if (SB(OARS(s)) <= 0) {
            if (B(OARF(s)) != 6) B(OARS(s))++;
            else { B(OARS(s)) = 0; B(OARREQ(s)) = 0; }
        } else {
            if (s == 1) B(SPEEDMSG) = 0x0A;
            nv_t_start_00D7(n);
            B(OARREQ(s)) = 0;
        }
    }
    B(GEAR(s)) = B(0x9818 + SB(OARS(s)) + 5 * B(TYPE(s)));
    if (s == 1) {
        if (B(GEAR(s)) != 0) music_play_5D29(B(0x9704 + SB(OARS(1))));
        else music_stop_5D20();
    }
}
/* B7FA(s): every 93FE[gear] ticks one step of 93F6[gear] px along the heading, fixed point 1/256; oar frame + 1 */
static void move_B7FA(int s) {
    int n = 8 * s + 2;
    int g = SB(GEAR(s));
    if (!(B(0x93FE + g) < nv_t_since_0183(n))) return;
    if (B(GEAR(s)) != 0) { B(OARF(s))++; if (SB(OARF(s)) > 6) B(OARF(s)) = 0; }
    int16_t step = SB(0x93F6 + g);
    int h = SB(HEAD(s));
    int16_t v = (int16_t)((int16_t)(sin_C4D8(h) * step) + SW(FX(s)));
    setW(FX(s), frac(v));
    setW(PX(s), (uint16_t)(SW(PX(s)) + div256(v)));
    v = (int16_t)((int16_t)(cos_C4E8(h) * step) + SW(FY(s)));
    setW(FY(s), frac(v));
    setW(PY(s), (uint16_t)(SW(PY(s)) + div256(v)));
    nv_t_start_00D7(n);
}
/* D38D(s) / D3BF(s): the push that keeps a ship inside y 15..170 / x 15..304 */
static int16_t push_y_D38D(int s) { int16_t y = (int16_t)SW(PY(s)); return y < 15 ? (int16_t)(15 - y) : y > 0xAA ? (int16_t)(0xAA - y) : 0; }
static int16_t push_x_D3BF(int s) { int16_t x = (int16_t)SW(PX(s)); return x < 15 ? (int16_t)(15 - x) : x > 0x130 ? (int16_t)(0x130 - x) : 0; }
/* D33D(side, axis, v): the ship pushed back is heading out of the screen (within 3 steps of 9928/992A[axis]): 9926[side]
   (enemy 8 = his admiral flees, Roman 4 = "swept apart") */
static uint16_t out_D33D(int side, int axis, int16_t v) {
    int t = v < 0 ? SB(0x9928 + axis) : SB(0x992A + axis);
    int d = t - SB(HEAD(side));
    d = d < 0 ? -d : d;
    return (d >= 4 && d <= 12) ? 0 : B(0x9926 + side);
}
/* D242: the screen edges. Only when the two ships are pushed in opposite directions at once does it return the mode
   bits; the pushes go to the positions AND (crosswise, literal) to the fractions, and their sum to every shot's */
static uint16_t edges_D242(void) {
    uint16_t m = 0;                                                  /* [bp-2] */
    int16_t di = push_y_D38D(1), si = push_y_D38D(0);
    if (di && si) {
        if (!((di ^ si) & 0x8000)) { if (si < di) si = 0; else di = 0; }
        else { m = out_D33D(1, 1, di); m |= out_D33D(0, 1, si); }
    }
    setW(PY(1), (uint16_t)(SW(PY(1)) + di));
    setW(0xC39A, (uint16_t)(W(0xC39A) + (uint16_t)(di << 8)));
    setW(PY(0), (uint16_t)(SW(PY(0)) + si));
    setW(0xC39E, (uint16_t)(W(0xC39E) + (uint16_t)(si << 8)));
    uint16_t a = (uint16_t)((uint16_t)(si << 8) + (uint16_t)(di << 8));
    setW(0xC3E6, (uint16_t)(W(0xC3E6) + a)); setW(0xC3E2, (uint16_t)(W(0xC3E2) + a));
    setW(0xC3EE, (uint16_t)(W(0xC3EE) + a)); setW(0xC3EA, (uint16_t)(W(0xC3EA) + a));
    di = push_x_D3BF(1); si = push_x_D3BF(0);
    if (di && si) {
        if (!((di ^ si) & 0x8000)) { if (si < di) si = 0; else di = 0; }
        else { m |= out_D33D(1, 0, di); m |= out_D33D(0, 0, si); }
    }
    setW(PX(1), (uint16_t)(SW(PX(1)) + di));
    a = (uint16_t)(di << 8);
    setW(0xC398, (uint16_t)(W(0xC398) + a));
    setW(PX(0), (uint16_t)(SW(PX(0)) + si));
    uint16_t c = (uint16_t)(si << 8);
    setW(0xC39C, (uint16_t)(W(0xC39C) + c));
    a = (uint16_t)(a + c);
    setW(0xC3E4, (uint16_t)(W(0xC3E4) + a)); setW(0xC3E0, (uint16_t)(W(0xC3E0) + a));
    setW(0xC3EC, (uint16_t)(W(0xC3EC) + a)); setW(0xC3E8, (uint16_t)(W(0xC3E8) + a));
    return m;
}
/* CD96: ramming (CE7E). The rammer C384 hits the other ship: damage rng(16) + 99BC[...] * oar level - 8 (halved on a
   scrape: headings within 2 steps), the hull and rng(damage) men; head on (6..10 steps apart) both. Returns the
   boarding bits 95D2 (enemy 1, Roman 2). 1990 bug 13.1: the damage was a byte (see the header) */
static uint16_t ram_CD96(void) {
    uint16_t di = 0;
    if (!touch_CE7E()) return 0;
    int R = W(RAMMER), V = R ^ 1;
    int8_t d8 = (int8_t)(B(HEAD(R)) - B(HEAD(V)));
    uint8_t dh = (uint8_t)(d8 < 0 ? -d8 : d8);                      /* [bp-2] */
    uint8_t idx = (uint8_t)(B(TYPE(R)) + (R == 1 ? (uint8_t)(3 * DIFF) : 6));
    uint16_t r16 = nv_rng(16);
    uint8_t dmg;                                                     /* [bp-1] */
    if (g_original_bugs) {
        dmg = (uint8_t)((uint8_t)r16 + (uint8_t)(B(0x99BC + idx) * B(OARS(R))) - 8);
        if ((int8_t)dh <= 2 || (int8_t)dh >= 14) dmg >>= 1;
    } else {
        int d = (int)r16 + B(0x99BC + idx) * B(OARS(R)) - 8;
        if (d < 0) d = 0;
        if ((int8_t)dh <= 2 || (int8_t)dh >= 14) d >>= 1;
        dmg = (uint8_t)d;
    }
    B(HULL(V)) = (uint8_t)(B(HULL(V)) - dmg);
    setW(CREW(V), (uint16_t)sub_050F(W(CREW(V)), nv_rng(dmg), 0));
    di = B(0x95D2 + R);
    if ((int8_t)dh >= 6 && (int8_t)dh <= 10) {
        B(HULL(R)) = (uint8_t)(B(HULL(R)) - dmg);
        setW(CREW(R), (uint16_t)sub_050F(W(CREW(R)), nv_rng(dmg), 0));
        di |= B(0x95D2 + V);
    }
    return di;
}
/* CC4D(s, step): the pot moves step px along its direction */
static void potmove_CC4D(int s, int16_t step) {
    int d = SB(PDIR(s));
    int16_t v = (int16_t)((int16_t)(sin_C4D8(d) * step) + SW(POTFX(s)));
    setW(POTFX(s), frac(v));
    setW(POTX(s), (uint16_t)(SW(POTX(s)) + div256(v)));
    v = (int16_t)((int16_t)(cos_C4E8(d) * step) + SW(POTFY(s)));
    setW(POTFY(s), frac(v));
    setW(POTY(s), (uint16_t)(SW(POTY(s)) + div256(v)));
}
/* CB11(s): the pot, every 7 ticks: splash frames 0x33..0x39; or 15 px on (gone off the field); at the end of its range
   it walks the last pixels with a hit test: a hit burns the other ship (98C0/98C4[diff] hull, rng(2 * dmg) men,
   fire rng(50) + 10), a miss splashes */
static void pot_CB11(int s) {
    int n = 8 * s + 1, hit = 0;
    if (!B(POT(s)) || !(nv_t_since_0183(n) > 6)) return;
    if (B(POT(s)) == 1) {
        B(POTANIM(s))++;
        if (SB(POTANIM(s)) > 0x39) B(POT(s)) = 0;
        nv_t_start_00D7(n);
        return;
    }
    B(POTANIM(s)) ^= 0x10;
    setW(PRANGE(s), (uint16_t)(W(PRANGE(s)) - 0x0F));
    int16_t ax = (int16_t)W(PRANGE(s));
    if (ax > 0) {
        potmove_CC4D(s, 0x0F);
        if (!(SW(POTX(s)) > 0 && SW(POTX(s)) < 0x13F && SW(POTY(s)) > 0 && SW(POTY(s)) < 0xB9)) B(POT(s)) = 0;
        nv_t_start_00D7(n);
        return;
    }
    uint16_t m = (uint16_t)(ax + 0x0F);                              /* [bp-2] */
    int o = s ^ 1;
    for (uint16_t si = 0; si < m; si++) {
        potmove_CC4D(s, 1);
        if (nv_d070(SW(POTX(s)), SW(POTY(s)), o)) { hit = -1; break; }
    }
    if (hit) {
        uint8_t dmg = o == 0 ? B(0x98C0 + DIFF) : B(0x98C4 + DIFF);
        B(HULL(o)) = (uint8_t)(B(HULL(o)) - dmg);
        setW(CREW(o), (uint16_t)sub_050F(W(CREW(o)), nv_rng((uint16_t)(2 * (int8_t)dmg)), 0));
        B(FIRE(o)) = (uint8_t)(nv_rng(0x32) + 0x0A);
        B(POT(s)) = 0;
    } else {
        B(POT(s)) = 1;
        B(POTANIM(s)) = 0x33;
    }
    nv_t_start_00D7(n);
}
/* C92B(s): an arrow volley at the other ship when it is closer than 90 and s has men; range max(distance - 10, 7) */
static void arrows_C92B(int s) {
    if (B(ARROW(s)) || B(POT(s)) || W(DIST) >= 0x5A || W(CREW(s)) == 0) return;
    B(ARROW(s)) = 0xFF;
    B(ADIR(s)) = B(BEAR(s));
    setW(ARRX(s), W(PX(s))); setW(ARRY(s), W(PY(s)));
    setW(ARRFX(s), W(0x56FE)); setW(ARRFY(s), W(0x5700));
    setW(ARANGE(s), (uint16_t)sub_050F((int16_t)W(DIST), 0x0A, 7));
}
/* C9A3(s): the volley, every 9 ticks 6 px; when its range is used up and it touches the other ship, 10 arrows hit with
   98C8 / 98CC[diff] % each */
static void arrows_C9A3(int s) {
    int n = 8 * s + 1;
    if (!B(ARROW(s)) || !(nv_t_since_0183(n) > 8)) return;
    if (SW(ARANGE(s)) <= 0) { B(ARROW(s)) = 0; nv_t_start_00D7(n); return; }
    setW(ARANGE(s), (uint16_t)(W(ARANGE(s)) - 6));
    int d = SB(ADIR(s));
    int16_t v = (int16_t)((int16_t)(sin_C4D8(d) * 6) + SW(ARRFX(s)));
    setW(ARRFX(s), frac(v));
    setW(ARRX(s), (uint16_t)(SW(ARRX(s)) + div256(v)));
    v = (int16_t)((int16_t)(cos_C4E8(d) * 6) + SW(ARRFY(s)));
    setW(ARRFY(s), frac(v));
    setW(ARRY(s), (uint16_t)(SW(ARRY(s)) + div256(v)));
    if (SW(ARANGE(s)) <= 0) {
        int o = s ^ 1;
        if (arrows_D11B(o)) {
            uint16_t p = o == 0 ? B(0x98C8 + DIFF) : B(0x98CC + DIFF), k = 0;
            for (uint16_t i = 0; i < 10; i++) if (nv_rng(0x64) < p) k++;
            setW(CREW(o), W(CREW(o)) > k ? (uint16_t)(W(CREW(o)) - k) : 0);
        }
    }
    if (!(SW(ARRX(s)) > 0 && SW(ARRX(s)) < 0x13F && SW(ARRY(s)) > 0 && SW(ARRY(s)) < 0xB9)) B(ARROW(s)) = 0;
    nv_t_start_00D7(n);
}

/* ---------------------------------------------------------------- the fleet battle */
/* DD2E(r): the Roman losses = C402 ships, spread over the types in proportion (remainders C3BE kept between rounds),
   then the legion aboard loses men per ship (cavalry share >> 2). 1990 bug 13.5: the products in 16 bits */
static void losses_DD2E(uint8_t r) {
    if (W(0xC404) == 0) return;
    uint16_t L = B(0x1117 + 12 * r), lost = 0;
    uint16_t di = (uint16_t)(0x110E + 12 * r);
    for (uint16_t si = REM(0); si < REM(3); si += 2, di += 2) {
        uint16_t n = W(di);
        uint16_t x = (uint16_t)((uint16_t)(n * W(0xC402)) + W(si));
        uint16_t q = (uint16_t)(x / W(0xC404));
        setW(si, (uint16_t)(x % W(0xC404)));
        if (n >= q) n = (uint16_t)(n - q); else { q = W(di); n = 0; }
        setW(di, n);
        lost = (uint16_t)(lost + q);
    }
    if (!lost) return;
    uint32_t t32 = (uint32_t)W(0xC08E) | (uint32_t)W(0xC090) << 16;
    t32 += lost; setW(0xC08E, (uint16_t)t32); setW(0xC090, (uint16_t)(t32 >> 16));
    if (W(0xC408) == 0) return;
    uint16_t cav = W(0x1920 + 20 * L), inf = W(0x191E + 20 * L), c;
    uint32_t t = (uint32_t)B(0x1118 + 12 * r) * lost;
    if (g_original_bugs) {
        t = (uint16_t)t;
        c = (uint16_t)((uint16_t)((uint16_t)t * cav) / W(0xC408) >> 2);
    } else {
        if (t > 0xFFFF) t = 0xFFFF;
        uint32_t c32 = (t * cav / W(0xC408)) >> 2;
        c = c32 > 0xFFFF ? 0xFFFF : (uint16_t)c32;
    }
    if (c <= cav) cav = (uint16_t)(cav - c); else { c = cav; cav = 0; }
    setW(0x1920 + 20 * L, cav);
    uint16_t m = (uint16_t)((uint16_t)t - c);
    if (m <= inf) inf = (uint16_t)(inf - m); else { m = inf; inf = 0; }
    setW(0x191E + 20 * L, inf);
    m = (uint16_t)(m + c);
    t32 = (uint32_t)W(0xC086) | (uint32_t)W(0xC088) << 16;
    t32 += m; setW(0xC086, (uint16_t)t32); setW(0xC088, (uint16_t)(t32 >> 16));
}
/* DE51(e): the enemy losses = C400 ships of the flagship's type only, then its army loses men per ship (the +0x0A
   byte's share >> 3, +4's >> 2, the rest from +6) */
static void losses_DE51(uint8_t e) {
    uint16_t A = B(0x1117 + 12 * e);
    uint16_t p = (uint16_t)(0x110E + 2 * (6 * e + B(TYPE(0))));
    uint16_t n = W(p), q = W(0xC400);
    if (n >= q) n = (uint16_t)(n - q); else { q = n; n = 0; }
    setW(p, n);
    if (!q) return;
    uint32_t t32 = (uint32_t)W(0xC08A) | (uint32_t)W(0xC08C) << 16;
    t32 += q; setW(0xC08A, (uint16_t)t32); setW(0xC08C, (uint16_t)(t32 >> 16));
    if (W(0xC40A) == 0) return;
    uint16_t a14 = (uint16_t)(14 * A);
    uint16_t s4 = W(0xC116 + a14), s6 = W(0xC118 + a14), sa = B(0xC11C + a14);
    uint32_t t = (uint32_t)B(0x1118 + 12 * e) * q;
    uint16_t a, b;
    if (g_original_bugs) {
        t = (uint16_t)t;
        a = (uint16_t)((uint16_t)(sa * (uint16_t)t) / W(0xC40A) >> 3);
    } else {
        if (t > 0xFFFF) t = 0xFFFF;
        a = (uint16_t)(((uint32_t)sa * t / W(0xC40A)) >> 3);
    }
    if (a <= sa) sa = (uint16_t)(sa - a); else { a = sa; sa = 0; }
    B(0xC11C + a14) = (uint8_t)sa;
    t = (uint16_t)((uint16_t)t - a);
    if (g_original_bugs) b = (uint16_t)((uint16_t)((uint16_t)t * s4) / W(0xC40A) >> 2);
    else { uint32_t b32 = (t * s4 / W(0xC40A)) >> 2; b = b32 > 0xFFFF ? 0xFFFF : (uint16_t)b32; }
    if (b <= s4) s4 = (uint16_t)(s4 - b); else { b = s4; s4 = 0; }
    setW(0xC116 + a14, s4);
    uint16_t m = (uint16_t)((uint16_t)t - b);
    if (m <= s6) s6 = (uint16_t)(s6 - m); else { m = s6; s6 = 0; }
    setW(0xC118 + a14, s6);
    m = (uint16_t)(m + a + b);
    t32 = (uint32_t)W(0xC082) | (uint32_t)W(0xC084) << 16;
    t32 += m; setW(0xC082, (uint16_t)t32); setW(0xC084, (uint16_t)(t32 >> 16));
}
/* DA50(e, r): one round: the legion aboard (6% per 500 men), every Roman ship (99BC[3 * diff + type], 99C8 after the
   Roman flagship sank), the Roman flagship's 99D4[diff] rolls at hull / 4 %; the enemy army aboard, the enemy ships of
   the flagship's type (the fixed row 99C2, 99CE after his flagship sank), his flagship's 4 rolls; each hit sinks a ship.
   -1 when one fleet is empty */
static uint16_t round_DA50(uint8_t e, uint8_t r) {
    setW(0xC402, 0); setW(0xC400, 0); setW(0xC406, 0); setW(0xC404, 0); setW(0xC40A, 0); setW(0xC408, 0);
    uint8_t L = B(0x1117 + 12 * r);
    if (L != 0xFF) {
        setW(0xC408, (uint16_t)(W(0x191E + 20 * L) + W(0x1920 + 20 * L)));
        for (uint16_t si = 0; W(0xC408) > si; si = (uint16_t)(si + 500)) if (nv_rng(0x64) <= 5) setW(0xC400, (uint16_t)(W(0xC400) + 1));
    }
    for (uint16_t t = 0; t < 3; t++) {
        uint16_t n = W(0x110E + 12 * r + 2 * t);
        setW(0xC404, (uint16_t)(W(0xC404) + n));
        uint16_t i = (uint16_t)(3 * DIFF + t);
        uint8_t p = B(0x99BC + i);
        if (B(HULL(1)) == 0) p = B(0x99C8 + i);
        for (uint16_t si = 0; n > si; si++) if (nv_rng(0x64) <= p) setW(0xC400, (uint16_t)(W(0xC400) + 1));
    }
    if (W(0xC404)) {
        uint16_t k = B(0x99D4 + DIFF);
        for (uint16_t si = 0; si < k; si++) if ((int16_t)nv_rng(0x64) <= SB(HULL(1)) / 4) setW(0xC400, (uint16_t)(W(0xC400) + 1));
    }
    uint8_t A = B(0x1117 + 12 * e);
    if (A != 0xFF) {
        setW(0xC40A, (uint16_t)(W(0xC116 + 14 * A) + W(0xC118 + 14 * A)));
        for (uint16_t si = 0; W(0xC40A) > si; si = (uint16_t)(si + 500)) if (nv_rng(0x64) <= 5) setW(0xC402, (uint16_t)(W(0xC402) + 1));
    }
    setW(0xC406, W(0x110E + 2 * (6 * e + B(TYPE(0)))));
    uint8_t p = B(0x99C2 + B(TYPE(0)));
    if (B(HULL(0)) == 0) p = B(0x99CE + B(TYPE(0)));
    for (uint16_t si = 0; W(0xC406) > si; si++) if (nv_rng(0x64) <= p) setW(0xC402, (uint16_t)(W(0xC402) + 1));
    if (W(0xC406))
        for (int si = 0; si < 4; si++) if ((int16_t)nv_rng(0x64) <= SB(HULL(0)) / 4) setW(0xC402, (uint16_t)(W(0xC402) + 1));
    if (W(0xC404)) losses_DD2E(r);
    if (W(0xC406)) losses_DE51(e);
    setW(0xC406, 0); setW(0xC404, 0);
    for (int i = 0; i < 3; i++) {
        setW(0xC404, (uint16_t)(W(0xC404) + W(0x110E + 12 * r + 2 * i)));
        setW(0xC406, (uint16_t)(W(0xC406) + W(0x110E + 12 * e + 2 * i)));
    }
    return (W(0xC404) && W(0xC406)) ? 0 : 0xFFFF;
}

/* ---------------------------------------------------------------- boarding */
/* BD41(courage, s): for each man of side s: rng(100) < courage, then rng(100) < 25 -> a kill */
static uint16_t melee_BD41(uint8_t courage, int s) {
    uint16_t k = 0;
    for (uint16_t i = 0; W(CREW(s)) > i; i++)
        if (nv_rng(0x64) < courage && nv_rng(0x64) < 0x19) k++;
    return k;
}
/* BB7E(bits, e, r): the boarding fight: a pause of 36 ticks, +10 courage to the side that boarded (13.2: the 1990 code,
   kept under --original, gave it to the other side: value 2 = the ROMAN boarded raised C36E, the enemy's), both crews fight in rounds rng(108) ticks apart until one is gone and 20 frames have passed
   (a frame every 9 ticks: both ships, smoke rng(3), the panel); then the message */
static void board_BB7E(uint16_t bits) {
    uint8_t n = 0x14;                                                /* [bp-1] */
    key_flush();                                                     /* 04ED */
    B(SPEEDMSG) = 0;
    nv_wait_011F(0x24);
    {   /* bit 95D2[s] = 1 << s marks side s as the boarder; 1990 bug 13.2 gave the +10 to the other side */
        int sw = g_original_bugs ? 1 : 0;
        if (bits & 2) B(COUR(1 ^ sw)) = (uint8_t)(B(COUR(1 ^ sw)) + 0x0A);
        if (bits & 1) B(COUR(0 ^ sw)) = (uint8_t)(B(COUR(0 ^ sw)) + 0x0A);
    }
    B(OARF(1)) = 6; B(OARF(0)) = 6;
    setW(ROUNDT, 0x48);
    int16_t cx = (int16_t)((int16_t)(SW(PX(1)) + SW(PX(0))) / 2);    /* [bp-4] */
    int16_t cy = (int16_t)((int16_t)(SW(PY(1)) + SW(PY(0))) / 2);    /* [bp-6] */
    for (;;) {
        if (!((W(CREW(0)) && W(CREW(1))) || n)) break;               /* bbdc */
        if (nv_t_since_0183(2) <= 8) { nv_tick(); continue; }
        nv_t_start_00D7(2);
        if (n) n--;
        if (nv_t_since_0183(8) > W(ROUNDT)) {
            nv_t_start_00D7(8);
            setW(ROUNDT, nv_rng(0x6C));
            uint16_t ke = melee_BD41(B(COUR(0)), 0);
            uint16_t kr = melee_BD41(B(COUR(1)), 1);
            setW(CREW(1), (uint16_t)sub_050F(W(CREW(1)), ke, 0));
            setW(CREW(0), (uint16_t)sub_050F(W(CREW(0)), kr, 0));
        }
        setW(0x9AFA, 9);
        nv_fill_7E74(0, 0, 0x13F, 0xB9);
        nv_ship_B8E3(1);
        nv_ship_B8E3(0);
        int k = nv_rng(3);
        nv_sprite_8E62(W(H_SMOK), k, cx, cy);
        nv_panel_C71B();
        nv_flip_6887();
    }
    if (W(CREW(0)) == 0) { B(HULL(0)) = 0; setW(0x96D2, W(0x96CA)); }
    if (W(CREW(1)) == 0) { B(HULL(1)) = 0; setW(0x96D2, B(HULL(0)) == 0 ? W(0x96CC) : W(0x96C8)); }
    nv_save_67B6();
    nv_page_6837();
    nv_font(0xA05C);
    setW(0x96CE, (bits & 2) ? W(0x96C4) : W(0x96C6));
    ds_strcpy(0xAC60, W(0x96CE));
    ds_strcat(0xAC60, W(0x96D0));
    ds_strcat(0xAC60, W(0x96D2));
    message_box(dstr(0xAC60));
    nv_font(0xA3B4);
}

/* ---------------------------------------------------------------- set-up and the end */
/* C274(r): "Choose a flag ship:" (DS:9750), one item per ship type the fleet has; no menu for one type; FF = Esc */
static uint8_t flagship_C274(uint8_t r) {
    uint8_t type_of[3] = { 0, 0, 0 };                                /* [bp-8] */
    int n = 0;
    uint16_t si = 0x972C;
    for (uint8_t t = 0; t < 3; t++) {
        if (!W(0x110E + 2 * (6 * r + t))) continue;
        type_of[n] = t;
        setW(si, W(0x9722 + 2 * t));
        B(si + 2) = B(0x9728 + t);
        si = (uint16_t)(si + 12);
        n++;
    }
    B(0x975E) = (uint8_t)n;
    int k = 0;
    if (n > 1) {
        setW(0x9756, n == 2 ? 0x76 : 0x82);
        menu_t m;
        menu_load_ds(&m, 0x9750);
        k = menu_choose(&m, 2, -1);                                  /* 5614(9750, 2), 54C8(9750, 0) */
        if (k < 0) return 0xFF;
    }
    if (n == 0) {                                                    /* 13.7: an empty Roman fleet */
        fprintf(stderr, "naval: Roman fleet %u has no ships at the flagship choice (the original reads an uninitialised local)\n", r);
        if (!g_original_bugs) return 0xFF;
    }
    return type_of[k];
}
/* BF50: NAVL: GALLEYS.LZW into the work buffer, OARS1 / OARS2; MNAVAL: (the picture to the screen) BOMBS, SMOK, the
   enemy's and the Roman's ship sheets, COLLIDE.LZW (5 tables), FLAG; music 13..16 loaded, 13 played. 0 = failed */
static uint16_t load_BF50(void) {
    uint16_t di = (uint16_t)nv_archive_0F7B(9), bp2;
    if (di) {
        if (nv_vga()) nv_galleys_0F0C();
        setW(H_OARS1, nv_sheet_163B(dstr(0x97D8)));
        setW(H_OARS2, nv_sheet_163B(dstr(0x97DE)));
        /* 11A9 */
    }
    di &= (uint16_t)nv_archive_0F7B(8);
    if (di) {
        bp2 = di;
        /* not VGA: galleys.cbm (missing from MNAVAL); the port is VGA */
        nv_restore_6973(0, 0, 0x13F, 0xC7);
        setW(H_BOMBS, nv_sheet_163B(dstr(0x97F0)));
        setW(H_SMOK, nv_sheet_163B(dstr(0x97F6)));
        setW(SHEET(0), nv_sheet_163B(dstr(W(0x980C + 2 * B(TYPE(0))))));
        setW(SHEET(1), B(TYPE(1)) != B(TYPE(0)) ? nv_sheet_163B(dstr(W(0x980C + 2 * B(TYPE(1))))) : W(SHEET(0)));
        if (!W(SHEET(0)) || !W(SHEET(1)) || !W(H_BOMBS) || !W(H_OARS1) || !W(H_OARS2) || !W(H_SMOK)) bp2 = 0;
        if (nv_collide_11C4(g_col, (int)sizeof(g_col))) {
            uint16_t dx = 0;
            for (int i = 0; i < 5; i++) { setW(W(0x970A + 2 * i), dx); dx = (uint16_t)(dx + B(0x9714 + i)); }
            di = bp2;
        } else di = 0;
        setW(H_FLAG, nv_sheet_163B(dstr(0x9807)));
        /* 11A9 */
    }
    for (int i = 1; i < 5; i++) music_load_5D73(B(0x9704 + i));
    music_play_5D29(0x0D);
    return di;
}
/* BE0F: the start of a duel (also on re-engage): enemy at (40, 40) heading east, Roman at (280, 160) heading west,
   cruising speed, everything in the air cleared; hulls and crews are kept */
static void start_BE0F(void) {
    B(HEAD(1)) = 0x0C; B(HEAD(0)) = 4;
    B(OARF(0)) = 6; B(OARF(1)) = 6;
    setW(PX(1), W(0x971A)); setW(PY(1), W(0x971C));
    setW(PX(0), W(0x971E)); setW(PY(0), W(0x9720));
    setW(FX(0), W(0x56FE)); setW(FY(0), W(0x5700));
    setW(FX(1), W(0x56FE)); setW(FY(1), W(0x5700));
    B(OARS(0)) = 1; B(OARS(1)) = 1;
    B(GEAR(1)) = B(0x9819 + 5 * B(TYPE(1)));
    B(GEAR(0)) = B(0x9819 + 5 * B(TYPE(0)));
    B(FIRE(0)) = 0; B(FIRE(1)) = 0; B(HOOKS(0)) = 0; B(HOOKS(1)) = 0; B(SPEEDMSG) = 0;
    B(OARREQ(0)) = 0; B(OARREQ(1)) = 0; B(TURNREQ(0)) = 0; B(TURNREQ(1)) = 0;
    B(POT(0)) = 0; B(POT(1)) = 0; B(ARROW(0)) = 0; B(ARROW(1)) = 0;
    setW(RAMMER, 1);
    setW(ROUNDT, 0xD8);
}
/* BD8A(e, r): music off and the map music unloaded; the flagship choice; C37A = the enemy's last type with ships; hulls
   8EE2[type]; BF50; the intro; BE0F. 0 = no battle */
static uint16_t setup_BD8A(uint8_t e, uint8_t r) {
    music_stop_5D20();
    music_unload_5E1A(1);
    B(TYPE(1)) = flagship_C274(r);
    if (B(TYPE(1)) == 0xFF) return 0;
    for (uint8_t t = 0; t < 3; t++) {
        setW(REM(t), 0);
        if (W(0x110E + 2 * (6 * e + t))) B(TYPE(0)) = t;
    }
    B(HULL(1)) = B(0x8EE2 + B(TYPE(1)));
    B(HULL(0)) = B(0x8EE2 + B(TYPE(0)));
    if (!load_BF50()) return 0;
    nv_intro_C105();
    start_BE0F();
    return 0xFFFF;
}
/* C312: the sheets freed (and the COLLIDE buffer, 74B7) */
static void free_C312(void) {
    if (W(SHEET(1)) != W(SHEET(0))) nv_sheet_19A0(W(SHEET(1)));
    nv_sheet_19A0(W(SHEET(0)));
    nv_sheet_19A0(W(H_OARS1));
    nv_sheet_19A0(W(H_OARS2));
    nv_sheet_19A0(W(H_BOMBS));
    nv_sheet_19A0(W(H_SMOK));
}
/* C34F: music off, cn9a..cn9d unloaded */
static void music_C34F(void) {
    music_stop_5D20();
    for (int i = 1; i < 5; i++) music_unload_5E1A(B(0x9704 + i));
}
/* B59F(outcome, mode, flags): "<Roman fleet> met the <enemy fleet> on the gently rolling waves of <sea>." + the flagship
   line (flags 1 Roman sank, 2 enemy sank, 3 both; mode 8 the admiral fled) + the result line; C0FE++ on a victory */
static void message_B59F(uint16_t outcome, uint16_t mode, uint16_t flags) {
    key_flush();                                                     /* 04ED */
    switch (flags) {
    case 0: setW(0x9356, 0x3E02); break;
    case 1: setW(0x9356, 0x8F52); break;
    case 2: setW(0x9356, 0x8EEE); break;
    case 3: setW(0x9356, 0x8F96); break;
    }
    if (mode == 8) setW(0x9356, 0x8FF2);
    switch (outcome) {
    case 1:
        setW(0x9358, (flags & 1) ? 0x913C : 0x9220);                 /* b5f9: "...witless commander..." when bit 0 clear */
        if (mode == 8) setW(0x9358, 0x90B0);
        break;
    case 2:
        setW(0xC0FE, (uint16_t)(W(0xC0FE) + 1));
        setW(0x9358, (flags & 1) ? 0x9104 : 0x9198);                 /* "their admiral's bath" / "Neptune's locker..." */
        if (mode == 8) setW(0x9358, 0x904C);
        break;
    case 3: setW(0x9358, 0x92B2); break;
    case 4: setW(0x9358, 0x9318); break;
    }
    nv_clip(0xC7);
    nv_font(0xA05C);
    ds_strcpy(0xAC60, W(0x934A));
    for (uint16_t p = 0x934C; p < 0x935A; p += 2) ds_strcat(0xAC60, W(p));
    message_box(dstr(0xAC60));
}

/* ---------------------------------------------------------------- the entries */
/* B0E0(Roman fleet f [bp+4], zone z [bp+6]), ret 4 (ported by the fleet agent; FLEET_NOTES section 11) */
uint16_t naval_enter_b0e0(uint8_t f, uint8_t z) {
    uint16_t si = 1;
    for (uint8_t e = 3; e < 6; e++) {                                         /* [bp-1] */
        uint16_t bx = (uint16_t)(12 * e);
        if (B(bx + 0x1114) != z || B(bx + 0x1116) == 0xFF) continue;         /* b0fe, b104 */
        /* FLEET_NOTES 9.3: the original starts B12A even when the Roman fleet is already gone (+8 = FFh after a result 3
           here or in 4ADA), with no ships. The port does not start that battle (the original's si = 1 is kept, so 2B40
           still writes its zones as in 1990); --original fights it. */
        if (!g_original_bugs && B(12 * f + 0x1116) == 0xFF) {
            fprintf(stderr, "naval: fleet %u is gone; the original would fight enemy fleet %u with no ships\n", f, e);
            break;
        }
        if (!(naval_battle_b12a(e, f) & 2)) { si = 0; break; }                /* b113: push f, e; test al, 2 */
    }
    return si;
}

/* B12A(enemy record e [bp+4], Roman fleet r [bp+6]), ret 4 */
uint16_t naval_battle_b12a(uint8_t e, uint8_t r) {
    if (!screen_ovl0()) {                                            /* --end-turn: no screen to fight on */
        g_enemy_unported = 1;
        fprintf(stderr, "naval: battle B12A(enemy record %u, Roman fleet %u) needs a screen (no window or script); result 0 (cancelled)\n", e, r);
        return 0;
    }
    /* the port's start-up state the EXE would have: the work buffer's descriptor (zero in a DS built from OVL0.OVL) and
       the screen as the target, as the map leaves it */
    if (W(0xC1DE) == 0) { memmove(&g_ds[0xC1DA], &g_ds[0x9AB6], 18); setW(0xC1DE, 0x2687); }
    nv_target_screen_680F();
    g_rt_on = 0;
    nv_burn_on = 0;

    uint16_t di = 0, si = 0, flags = 0, mode = 0, key = 0, save4 = 0, res;   /* [bp-0Ah] flags, [bp-8] mode, [bp-2], [bp-4] */
    /* 1D55(0): the cursor (not ported) */
    if (!setup_BD8A(e, r)) return 0;                                 /* b14f jmp b592 */
    res = si;                                                        /* [bp-0Ch] */
    setW(0xC0FC, (uint16_t)(W(0xC0FC) + 1));
    uint16_t e6 = (uint16_t)(6 * e), r6 = (uint16_t)(6 * r);         /* [bp-0Eh], [bp-14h] */
    setW(0x110E + 2 * (e6 + B(TYPE(0))), (uint16_t)(W(0x110E + 2 * (e6 + B(TYPE(0)))) - 1));   /* b174 */
    setW(CREW(0), B(0x8EDE + B(TYPE(0))));
    uint8_t A = B(0x1117 + 12 * e);
    B(COUR(0)) = A != 0xFF ? B(0xC11E + 14 * A) : B(0x527A + 32 * B(0x1115 + 12 * e));
    B(COUR(0)) = (uint8_t)(B(COUR(0)) + B(0x8EE6 + DIFF));
    setW(0x110E + 2 * (r6 + B(TYPE(1))), (uint16_t)(W(0x110E + 2 * (r6 + B(TYPE(1)))) - 1));   /* b1de */
    setW(CREW(1), B(0x8EDE + B(TYPE(1))));
    uint8_t L = B(0x1117 + 12 * r);
    if (L != 0xFF) B(COUR(1)) = B(0x1923 + 20 * L);
    else { setW(CREW(1), (uint16_t)(W(CREW(1)) >> 1)); B(COUR(1)) = B(0x527A + 32 * B(0x1115 + 12 * r)); }
    B(COUR(1)) = (uint8_t)(B(COUR(1)) + B(0x8EEA + DIFF));
    nv_screen_BEAE(e, r);
    nv_clip(0xB9);
    nv_burn_on = 1;
    for (int first = 1;; first = 0) {                                /* b248 */
        if (!first) nv_tick();                                       /* the port: one pass per tick */
        if (di) break;
        setW(DIST, dist_C4F8((int16_t)SW(PX(0)), (int16_t)SW(PY(0)), (int16_t)SW(PX(1)), (int16_t)SW(PY(1))));
        B(BEAR(1)) = bearing_C641();
        B(BEAR(0)) = (uint8_t)((B(BEAR(1)) - 8) & 0x0F);
        key = input_C366();
        ai_B6A0();
        turn_C3D3(1); turn_C3D3(0);
        oars_C422(1); oars_C422(0);
        move_B7FA(1); move_B7FA(0);
        if (mode == di) mode = edges_D242();                         /* b2ad: [bp-8] == di (0) */
        rect_CF7F(1); rect_CF7F(0);
        front_CFDD();
        si = ram_CD96();
        pot_CB11(1); pot_CB11(0);
        arrows_C92B(1); arrows_C92B(0);
        arrows_C9A3(1); arrows_C9A3(0);
        if (nv_t_since_0183(8) > W(ROUNDT)) {                        /* b2f4: a background round of the fleet battle */
            nv_t_start_00D7(8);
            round_DA50(e, r);
            setW(ROUNDT, (uint16_t)((nv_rng(9) + 2) * 0x48));
        }
        if (nv_t_since0_01AE() > 2) {                                /* b324: a frame */
            nv_t_reset_00F8();
            setW(0x9AFA, 9);
            nv_fill_7E74(0, 0, 0x13F, 0xB9);
            nv_ships_B96A();
            si |= nv_hooks_BA7A(1);
            si |= nv_hooks_BA7A(0);
            nv_shots_CCDE(1); nv_shots_CCDE(0);
            nv_panel_C71B();
            nv_totals_DC8E(e, r);
            nv_flip_6887();
        }
        if (sunk(1) || sunk(0)) { di = 0xFFFF; continue; }           /* b376 (13.3: <= 0 unless --original) */
        if (si) {
            music_play_5D29(0x10);                                   /* cn9d */
            board_BB7E(si);
            di = 0xFFFF; continue;
        }
        save4 = di;                                                  /* b3a0: [bp-4] */
        if (key != di) { di = 0xFFFF; mode = key; continue; }
        if (mode & 4) {                                              /* b3b4: the Roman ran out of the screen */
            music_play_5D29(0x0D);
            nv_save_67B6();
            nv_page_6837();
            setW(0x9AFA, 9);
            nv_fill_7E74(0, 0, 0x13F, 0xB9);
            nv_font(0xA05C);
            if (!yes_no(dstr(0x935A))) {                             /* "The tides of battle sweep..." No: b3f3 je b39a */
                /* NAVAL_SCREEN_NOTES 9.2: the original leaves the box font A05C set, and b479's C71B prints the MEN line
                   in it (8-px glyphs from y 194, cut off at 199). BB7E puts A3B4 back after its box (bd2c); so does the
                   port here unless --original. */
                if (!g_original_bugs) nv_font(0xA3B4);
                di = 0xFFFF; continue;
            }
            mode = 0;
            start_BE0F();
            nv_screen_BEAE(e, r);
            di = save4;
            continue;
        }
        if (mode & 8) { di = 0xFFFF; continue; }                     /* the enemy admiral fled */
    }
    /* b419 */
    nv_burn_on = 0;
    di = flags;
    music_play_5D29(0x0D);
    if (sunk(1)) {
        di = 1;
        uint32_t t = ((uint32_t)W(0xC08E) | (uint32_t)W(0xC090) << 16) + 1; setW(0xC08E, (uint16_t)t); setW(0xC090, (uint16_t)(t >> 16));
    }
    if (sunk(0)) {
        di |= 2;
        uint32_t t = ((uint32_t)W(0xC08A) | (uint32_t)W(0xC08C) << 16) + 1; setW(0xC08A, (uint16_t)t); setW(0xC08C, (uint16_t)(t >> 16));
    }
    if (!(di & 1)) setW(0x110E + 2 * (r6 + B(TYPE(1))), (uint16_t)(W(0x110E + 2 * (r6 + B(TYPE(1)))) + 1));   /* b450 */
    if (!(di & 2)) setW(0x110E + 2 * (e6 + B(TYPE(0))), (uint16_t)(W(0x110E + 2 * (e6 + B(TYPE(0)))) + 1));   /* b465 */
    B(SPEEDMSG) = 0;
    nv_panel_C71B();
    nv_save_67B6();
    nv_sink_D3F2(di);
    free_C312();
    nv_sink_D60C(di);
    nv_save_67B6();
    nv_page_6837();
    flags = di;
    if (mode == 8) B(HULL(0)) = 0;                                   /* b493: the fled admiral's fleet fights as if he sank */
    if (sunk(1) || sunk(0)) {                                        /* b49e */
        while (!round_DA50(e, r)) ;                                  /* b4ac: the fleet battle to the end */
        nv_totals_DC8E(e, r);
    }
    uint16_t rs = 0, es = 0;                                         /* [bp-6] (dx), di */
    for (int i = 0; i < 3; i++) {
        rs = (uint16_t)(rs + W(0x110E + 12 * r + 2 * i));
        es = (uint16_t)(es + W(0x110E + 12 * e + 2 * i));
    }
    si = res;
    if (rs && es) si = 4;                                            /* b50e: both left: retreat */
    else {
        if (!rs) {                                                   /* b513: the Roman fleet is gone: lost */
            B(0x1116 + 12 * r) = 0xFF;
            if (B(0x1117 + 12 * r) != 0xFF) legion_lost(B(0x1117 + 12 * r));   /* 4816 */
            si = 1;
        }
        if (!es) {                                                   /* b536: the enemy fleet is gone: won */
            B(0x1116 + 12 * e) = 0xFF;
            uint8_t a = B(0x1117 + 12 * e);
            if (a != 0xFF) setW(0xC11A + 14 * a, es);
            si |= 2;
        }
    }
    setW(0x934A, W(0x1100 + 2 * r));
    setW(0x934E, W(0x1100 + 2 * e));
    setW(0x9352, W(0x5222 + 2 * B(0x1114 + 12 * e)));
    message_B59F(si, mode, flags);
    music_C34F();
    /* b592: 1D8A, the cursor (not ported) */
    if (!g_original_bugs) nv_palette_restore();
    fprintf(stderr, "naval: battle of fleet %u against record %u: result %u (flagships sunk %u, mode %u), ships Rome %u, enemy %u\n",
            r, e, si, flags, mode, rs, es);
    return si;
}
