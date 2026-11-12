/* The chariot race and the victory sequence, OVL0 section 4 (file 0x51A0-0x8550, loaded at 0823:B0E0), Claude 2026-11-12.
 * Translated literally from disasm/OVL_RACE.asm (addresses in the comments), after docs/RACE_INDEX.md, RACE_LOGIC_NOTES.md,
 * RACE_SCREEN_NOTES.md and RACE_DATA_NOTES.md. The drawing routines are in race_gfx.c (on naval_gfx.c's EXE helpers).
 *
 *   D08A  the race (entry, thunk B0AA from Rome Games 0x349D)    DF58  the victory (entry, thunk B092 from the map 0x09A4)
 *   C2DE  pre-race: sheets, field (DA5A), cards, the chariot purchase, "How Shall We Prepare?" (C730 skulduggery, C88E bet),
 *         BE10 the gallery parade, the race screen, CBE6 the path        C9B6  the result, BF6A the winner's parade
 *   one race tick (7 timer ticks): DABA opponents, CECC the player's input, D17A curves and skids, B190 movement (D5E1 probes,
 *   BAC5 screen y, B6CB crash state, B460 the state machine with its bumps B8BC / B977 / B9C9 / D53E / D590, BD0A the sprite
 *   trees), the horses' rest, the gauges D3C2; D738 the player's crash scene          DF74  the victory statistics
 *
 * TIME. As the naval battle and the gladiator show: the original's clock is kept in the DS (DS:42/44 and the slots DS:46..,
 * naval.c's nv_t_* = EXE 00D7 / 00F8 / 01AE / 011F) and every busy-wait runs naval.c's nv_tick(): one tick per call, in the
 * window at 72.8 Hz real time, headless (--script) at once. A race tick is exactly 7 timer ticks (d12b), as are the parade
 * and crash-scene frames, so a race is a pure function of the dice and the input per race tick. Input in the race (CECC,
 * once per race tick): one key read (03AA) and dropped, the mouse (0644: the motion steers, the buttons whip / lash), and the
 * held arrows DS:A2 (screen_held_dirs; script "hold KEY N" holds a key for N timer ticks = N / 7 race ticks, "wait N" waits
 * N ticks): Up = whip (only after a race tick with no action), Up+Left / Up+Right (Home / PgUp) = lash left / right,
 * Down = rein in, Left / Right = steer in / out. Script "click X Y" in the race = the left button = lash left; "key" lines
 * end the parades (any key) and the crash scene (only "key esc").
 *
 * 1990 BUGS fixed by default (--original keeps each):
 *   1. Finish ticks re-stamped (LOGIC 11.1): b315 tests +0D == 7, but B460 overwrites +0D at every animation boundary, so a
 *      finished racer's tick was stamped again and again; opponents who finished before the player were then ranked by
 *      animation phase (and the wrong province could get the winner's -10 unrest). Fix: stamped only once (FIN == FFFF).
 *   2. Unfinished racers ranked by the position within the lap, laps ignored (11.2, ca29): a racer a lap behind could rank
 *      first when nobody finished. Fix: laps first, then the position.
 *   3. The tie-break rng_roll(1) is always 0 (11.3, ca4d): a full tie always swapped. Fix: rng_roll(2) (the same dice count).
 *   4. Probe results overflow (11.4, D5E1 d6e2/d70a): up to 6 hits were written into a probe's 2 slots, spilling into the
 *      next probe's slots and, from probe 5, into C350 (the leader's lap: the lap counter froze) and C352 (the "med" sheet
 *      pointer). Fix: at most 2 hits per probe. --original spills up to C351 but never past it (C352 is a sheet value here).
 *   5. Ramming the chariot in front hit the wrong racer (11.5, b504): B977(front, 5) read the moving racer's REAR probe, so
 *      with nobody behind it became B9C9(front, 0, 0): Red's horses lost strength whoever rammed whom. Fix: B9C9(front, i, 0)
 *      (the racer that rammed is hit, as b4ee hits the one behind; the same two dice).
 *   6. Crashing opponents steered by the PLAYER's speed (11.6, b72c reads C2FE for every racer). Fix: their own speed (the
 *      rng(10) of the speed-up branch then follows the opponent's speed).
 *   7. The gods' favour was taken back at the first rest (11.7, d0fd): add_cap(horses, 1, max) set 59 back to 40 (the gauge
 *      divides by 60). Fix: a rest adds 1 only below the maximum (the rest clock and the gauge flag as before).
 *   8. The opponents' skill worked backwards (11.8, dc3b..dc77): the clean line was taken when rng(100) >= skill (5 % of the
 *      time at skill 95) and the random error, always downward, grew with the skill, so a better field drove slower at
 *      Galley Slave and Legionnaire; every C9B6 adjustment reads skill as "% chance of a clean line". Fix: the clean line
 *      when rng(100) < skill, else the error with a = rng(100) - skill (the dice per tick differ, they cannot be kept).
 *   9. Galley Slave "Cautious" cap wrapped below 0 (11.9, dcaa: base + rng(a) - 3a/4 in a byte -> 238..255 = no cap). Fix:
 *      the cap is at least 20 (no dice change; moot after fix 8).
 *  10. Uninitialised maximum in DABA (11.11, [bp-12h]): fix 0. --original models the stack word as the notes trace it:
 *      0xD499 (left by the gauges) in normal ticks, the y of the last minimap dot while the player crashes (G).
 *  11. NEW (found by this port, D): B8BC's horse bump indexed the victim's record with the UNMASKED probe byte (b914: ax =
 *      j | 0x10, si = 18 * ax, di = ax): the "spooked" surge rng(10) went into DS byte C3E8 + 18j and the rest clock C322
 *      into the probe slots C344 + 2j (re-made before use), so the victim never surged and its rest clock never restarted
 *      (only D590's -1 horse strength, masked, worked). Fix: j = e & 15. --original writes those DS bytes as 1990 did.
 *  12. The victory statistics' singulars (11.18): only "alliance(s)" and "one fleet battle" had them; "won 1 battles",
 *      "lost 1 battles", "won 1 fleet battles", "sponsored 1 games", "entered 1 races", and with the verb "1 legionnaires
 *      were killed", "1 enemy soldiers\nwere slain", "1\nof your ships were sunk", "1 enemy ships were sunk", "1 talents in
 *      tribute / bets / plunder" (e088 e0b5 e1a6 e1d3 e2bd e2ea e317). Fix: the singular (and "was") for 1.
 *  13. NEW (D for the effect, G for the intent; seen in headless runs): a finished racer drives on and its lap count goes
 *      on past 3 (b2be), but B190 counts the racers still racing, and draws their minimap dots, by laps != 3 (b3f0): a
 *      finished opponent who completes another lap counts as racing again (its dot comes back), so "only the player is
 *      left" never comes and the race runs until the player finishes (5806 race ticks in a --original run at Galley
 *      Slave), and the leader's lap C350 goes past 3, so D392 draws laps frames 4, 5, ... past the end of the 4-frame
 *      LAPS sheet (whatever follows the descriptors in 1990; nothing in the port). Fix: racing = laps < 3, and the lap
 *      counter frame at most 3.
 * Kept (D, quirks or design): the 3rd place lengthening the odds like a loss (cb41 falls into cb48); the Emperor without
 * skill drift (cb61, no case 3; skill is not used at Senator / Emperor); the dead leader branch (dbfd); the one-point gaps of
 * the curve zones; the step frozen in the curve approach; a crashing chariot that can still finish; the bribe that may do
 * nothing (top - rng(10)); "talents in bets" counted gross; the sticking keyboard direction bits; the empty "Click to
 * continue" box after the purchase (DS:3E02 = ""); 585B with FF after Esc (nothing drawn here); the node counters BD0A
 * leaves running between races.
 * NOT a fix, Erik's call (D, found here; the notes say "2 ticks per frame"): BD0A's animation counter is inverted
 * (bd6a `cmp [si+0Eh], al; ja bd82` skips the reset when tick > per), so with per = 1 or 2 every active node advances one
 * frame per race tick and the per byte does nothing: the chariot's 4-frame cycle reaches frame 0 every 4 ticks, so whips,
 * lashes and the speed cap at top come every 4 race ticks (0.4 s), not every 8 (horses per 1) or 12 (drivers per 2). The
 * game was balanced with this, so it is kept in both modes.
 * Also Erik's call (open decisions, the code unchanged until he rules):
 *   - Fix 8 is the default, but its intent is G and it changes the difficulty a lot: with it the Galley Slave /
 *     Legionnaire field corners almost as well as Senator's (RACE_LOGIC_NOTES 12.5), with the 1990 code far slower, so
 *     the two easy levels nearly collapse into Senator in the curves. It also changes the dice per tick, so the two
 *     readings cannot be swapped on one dice stream. Options: keep it, or keep 1990 by default (as BD0A above).
 *   - The bribe text (W(881E)) "driver has been paid\n10 talents to let you win." contradicts the code (G: joke or slip):
 *     the bribed driver loses rng(10) = 0..9 top speed (c7d4..c7e5) and stays at 101..110, above the player's 100, and
 *     the AI is otherwise unchanged. A fix would need a design (none proposed; RACE_LOGIC_NOTES 11.15).
 *
 * PORT ADDITIONS: the mouse cursor (1D19 / 1CCE ...) and the disk prompt 1E21 are not drawn; music is the DS part only
 * (map.c / naval.c). The credits of the victory sequence (section 1 B9A1, thunk B0CE) are in credits.c. When
 * the input ends (window closed, script run out): at a pre-race menu the race is cancelled as by "Exit" + Yes (C70F); in the
 * parades and the crash scene as by a key; in the race the race stops and the result (C9B6) is skipped (154E is written
 * and the bet paid, as d09e / c5bd come before). The original has no way out of the race.
 *
 * Easter eggs kept (all the data is the game's): the physician's herbs "'guaranteed' to make your horses win.  You doubt that
 * this will work" (top + rng(10): one time in ten nothing), the squinty-eyed attendant of the Vestal Virgins and her "small
 * donation of 50 talents" (the only sure trick), the bribed driver "paid 10 talents to let you win" who loses only 0..9 of
 * his 110 top speed (still faster than your 100) and the bribes going Red, Green, Blue in that order; "Utter, Ignominious
 * Failure"; Agrippa's dolphins tipping over as the lap counter; the stretcher bearers with a red cross, in two endings (the
 * driver carried off, or waving them away while they leave with the empty stretcher); the minimap dots hiding behind the
 * spina's monuments; the winner raising both arms, and in the triumph the figure in red beside him; the epilogue "like
 * those of Alexander ... nothing is forever"; the unused GALLERY2.CBM in MRACE (never loaded, as in 1990); the end credits
 * (credits.c, DS:6A6E: 18 title / names pairs) closing with an untitled "Beware the ides of March" (DS:6AB6 -> 69FB "",
 * 69FC) after "Artist Photo / Bill Fiesterman".
 *
 * NOT VERIFIED against DOSBox: no capture of section 4 exists except one pre-race parade frame (RACE_SCREEN_NOTES 4); all
 * else is D (decoded) and checked by headless runs.
 */
#include <stdio.h>
#include <string.h>
#include "state.h"
#include "screen.h"
#include "naval_int.h"
#define RACE_INTERNAL
#include "race.h"

void music_load_5D73(uint8_t m);          /* map.c (the DS part of 5D73) */
void gl_box_3233(int x0, int y0, int x1, int y1, int c);   /* glad_gfx.c: EXE 3233 (glad.h's names clash with race.h's) */
void province_frame30e6(int x, int y);    /* province.c: 0x30E6 on the screen */
void province_panel3512(void);            /* province.c: 0x3512 on the screen */
void province_rome_panel6f0c(uint8_t z);  /* province.c: 0x6F0C(z) on the screen */
void province_treasury44e5(uint16_t v);   /* province.c: 0x44E5(v) on the screen */
void credits_B9A1(void);                  /* credits.c: section 1 B9A1, the end credits */

static int g_input_gone;                  /* port: the input ended in the race */
static uint16_t g_dot_y = 0xD499;         /* --original model of DABA's [bp-12h] while the player crashes (bug 10) */

static const char *dstr(uint16_t a) { return (const char *)&g_ds[a]; }
static void msg_59D4(uint16_t s) { message_box(dstr(s)); }
static size_t ds_len(uint16_t a) { size_t n = 0; while ((size_t)a + n < 0x10000 && g_ds[a + n]) n++; return n; }
static void ds_cat(uint16_t d, const char *s) {                  /* 9DF2 */
    size_t at = (size_t)d + ds_len(d), n = strlen(s);
    if (at + n + 1 > 0x10000) n = at < 0xFFFF ? 0xFFFF - at : 0;
    memmove(&g_ds[at], s, n);
    if (at + n < 0x10000) g_ds[at + n] = 0;
}
static void ds_cpy(uint16_t d, const char *s) { char t[1024]; snprintf(t, sizeof(t), "%s", s); g_ds[d] = 0; ds_cat(d, t); }   /* 9E32 */
static void ds_cat_num(uint16_t d, long v) { char b[24]; snprintf(b, sizeof(b), "%ld", v); ds_cat(d, b); }   /* 9FDC / A022 + 9DF2 */
static int sub_050F(int v, int step, int lim) { int r = (int16_t)(v - step); return r < lim ? lim : r; }

/* ---------------------------------------------------------------- music (the DS part, as naval.c / glad.c) */
static void music_stop_5D20(void) { B(0x3EEE) = 0xFF; }
static void music_unload_5E1A(uint8_t n) {
    for (int i = 0; i < 4; i++) {
        if (B(0xA6B2 + 4 * i) == 0 || B(0xA6B3 + 4 * i) != n) continue;
        setW((uint16_t)(0xA6B0 + 4 * i), 0); B(0xA6B2 + 4 * i) = 0; B(0xA6B3 + 4 * i) = 0xFF; B(0x3EEE) = 0xFF;
        break;
    }
}

/* ---------------------------------------------------------------- menus (5614 / 54C8 / 585B on the screen) */
static void menu_draw_5614(uint16_t rec, int mode) {
    menu_t m;
    menu_load_ds(&m, rec);
    if (mode == 1) { province_frame30e6(m.x, m.y); ui_menu_draw_mode(g_scr, &m, 0, -1); }
    else ui_menu_draw_mode(g_scr, &m, mode, -1);
}
static uint16_t menu_run_54C8(uint16_t rec) {
    menu_t m;
    menu_load_ds(&m, rec);
    int r = menu_choose(&m, -1, -1);
    return r < 0 ? 0xFF : (uint16_t)r;
}
static void menu_mark_585B(uint16_t rec, uint16_t item, int on) {
    menu_t m;
    menu_load_ds(&m, rec);
    ui_menu_highlight(g_scr, &m, item, on);                          /* FF (Esc): nothing (G for 1990) */
}

/* ---------------------------------------------------------------- the sprite trees */
/* BCA0(&n, frames, node): pre-order: node +4 = race sheet + 14 * frames[n], +A = DS:83A4[n] */
static void number_BCA0(uint8_t *n, uint16_t frames, uint16_t node) {
    for (uint16_t si = node; si; si = W(si + N_NEXT)) {
        setW(si + N_DESC, (uint16_t)(0xE * B(frames + *n) + W(0xC2BC)));
        setW(si + N_ACT, W(0x83A4 + 2 * *n));
        (*n)++;
        if (W(si + N_CHILD)) number_BCA0(n, frames, W(si + N_CHILD));
    }
}
/* BD0A(node, x, y): each active sibling drawn at (x + dx + otab[f].dx, y + dy + otab[f].dy), then its counter, then its
   children at that point. The counter as the bytes have it (see the header): tick++; if (tick <= per) { tick = 0; frame++
   (wrapping at the count) } */
static void tree_BD0A(uint16_t node, int16_t x, int16_t y) {
    for (uint16_t si = node; si; si = W(si + N_NEXT)) {
        if (W(si + N_ACT) == 0) continue;
        uint8_t f = B(si + N_FR);
        uint16_t ot = (uint16_t)(W(si + N_OTAB) + 2 * f);
        int16_t dx = (int16_t)(SB(si + N_DX) + SB(ot) + x);
        int16_t dy = (int16_t)(SB(si + N_DY) + SB(ot + 1) + y);
        rc_sprite_8E62((uint16_t)(f * 0xE + W(si + N_DESC)), dx, dy);
        B(si + N_TK)++;
        if (!(B(si + N_TK) > B(si + N_PER))) {                       /* bd6d: ja bd82 */
            B(si + N_TK) = 0;
            B(si + N_FR)++;
            if (!(B(si + N_CNT) > B(si + N_FR))) B(si + N_FR) = 0;
        }
        if (W(si + N_CHILD)) tree_BD0A(W(si + N_CHILD), dx, dy);
    }
}
/* BBAD(type, i): the chariot node's frames (83C4[type]), the crash node's frames (83C8) and offsets (83E4); chariot on,
   crash off */
static void tree_type_BBAD(uint8_t type, int i) {
    uint16_t ch = W(ROOT(i)), cr = W(ch);
    setW(ch + N_DESC, (uint16_t)(0xE * B(0x83C4 + type) + W(0xC2BC)));
    setW(cr + N_DESC, (uint16_t)(0xE * B(0x83C8 + type) + W(0xC2BC)));
    setW(cr + N_OTAB, W(0x83E4 + 2 * type));
    setW(ch + N_ACT, 0xFFFF);
    setW(cr + N_ACT, 0);
}
/* BA28(i, k): driver pose k (0 reins, 1 whip, 2 / 3 lash): the four pose nodes under the chariot off, the k-th on; nothing
   when the last action is k, 11, or 4..7 (the "all off" loop BA71 for 11 is unreachable) */
static void pose_BA28(int i, uint16_t k) {
    uint8_t last = B(REC(i, R_LAST));
    if (last == (uint8_t)k || last == 0xB) return;
    uint16_t first = W(W(ROOT(i)) + N_CHILD);                        /* [bp-2] = [bp-4] */
    if (last >= 4 && last <= 7) return;
    uint16_t p = first;
    for (int dx = 0; dx <= 3; dx++) {                                /* ba8c */
        setW(p + N_ACT, 0);
        if (W(p)) p = W(p);
    }
    uint16_t bx = first;
    for (uint16_t dx = 0; dx < k; dx++) if (W(bx)) bx = W(bx);       /* baac */
    setW(bx + N_ACT, 0xFFFF);
}

/* ---------------------------------------------------------------- speckles */
/* B0E0: the 10 speckles DS:AA78 (x, y, pattern): x = 5 + 4 rng(77), y = 45 + rng(109), pattern = rng(65535) */
static void speckles_B0E0(void) {
    for (uint16_t si = 0xAA7A; si < 0xAAB6; si += 6) {
        setW(si - 2, (uint16_t)((rng_roll(0x4D) << 2) + 5));
        setW(si, 0x2D);
        setW(si, (uint16_t)(rng_roll(0x6D) + 0x2D));
        setW(si + 2, rng_roll(0xFFFF));
    }
}
/* B11C(scroll): scroll != 0: acc = [83EA] + speed, dy = acc >> 4, [83EA] = acc & 15; plot each (DE64), y += dy, past
   y 170 a new one at y 45 */
static void speckles_B11C(int scroll) {
    uint16_t di;
    if (scroll) {
        setW(0x83EA, (uint16_t)(W(0x83EA) + B(0xC2FE)));
        di = (uint16_t)(W(0x83EA) >> 4);
        setW(0x83EA, (uint16_t)(B(0x83EA) & 0xF));
    } else di = 0;
    for (uint16_t si = 0xAA7A; si < 0xAAB6; si += 6) {
        rc_plot_DE64(W(si + 2), SW(si), SW(si - 2));
        setW(si, (uint16_t)(W(si) + di));
        if (SW(si) > 0xAA) {
            setW(si - 2, (uint16_t)((rng_roll(0x4D) << 2) + 5));
            setW(si, 0x2D);
            setW(si + 2, rng_roll(0xFFFF));
        }
    }
}
/* DA2E: the view cleared (0x11 VGA) and the speckles drawn without moving */
static void clear_DA2E(void) {
    setW(0x9AFA, 7);
    if (nv_vga()) setW(0x9AFA, 0x11);
    nv_fill_7E74(0, 0x3D, 0x13F, 0xAA);
    speckles_B11C(0);
}

/* ---------------------------------------------------------------- probes, speeds, steering, bumps */
static void put_probe(int k, int n, uint8_t v) {
    uint16_t a = (uint16_t)(0xC344 + 2 * k + n);
    if (!g_original_bugs) { if (n < 2) B(a) = v; return; }           /* bug 4 */
    if (a <= 0xC351) B(a) = v;                                       /* 1990 went on into C352..: never in the port */
}
/* D5E1(i): the six probes of racer i (DS:8AB4) against the horses (8AE4) and chariot (8AEC) rects of every other running
   racer: C344 + 2k + n = j | 10h (horses) / j | 20h (chariot) */
static void sides_D5E1(int i) {
    for (int k = 0; k < 12; k++) B(0xC344 + k) = 0;
    int16_t A[4][4], Bc[4][4];
    for (int j = 0; j < 4; j++)
        for (int q = 0; q < 4; q++) {
            int16_t o = (q & 1) ? SW(SCRY(j)) : SW(LANE(j));
            A[j][q] = (int16_t)(SW(0x8AE4 + 2 * q) + o);
            Bc[j][q] = (int16_t)(SW(0x8AEC + 2 * q) + o);
        }
    int16_t px = SW(LANE(i)), py = SW(SCRY(i));
    for (int k = 0; k < 6; k++) {
        int16_t pr[4];
        for (int q = 0; q < 4; q++) pr[q] = (int16_t)(SW(0x8AB4 + 8 * k + 2 * q) + ((q & 1) ? py : px));
        int n = 0;
        for (int j = 0; j < 4; j++) {
            if (j == i || W(REC(j, R_RUN)) == 0) continue;
            if (nv_rect_817C(NULL, pr, A[j])) put_probe(k, n++, (uint8_t)((j & 0xF) | 0x10));
            if (nv_rect_817C(NULL, pr, Bc[j])) put_probe(k, n++, (uint8_t)((j & 0xF) | 0x20));
        }
    }
}
/* B7D2(i, n): blocked in front: 0; speed = max(speed + n, 20) (no cap here) */
static int accel_B7D2(int i, uint16_t n) {
    if (B(0xC34C)) return 0;
    uint16_t v = (uint16_t)(B(REC(i, R_SPEED)) + n);
    if (v < 0x14) v = 0x14;
    B(REC(i, R_SPEED)) = (uint8_t)v;
    if (i == 3) setW(0xC32E, 0xFFFF);
    return -1;
}
/* B810(i): pushed from behind: 0; speed = max(speed - 5, 20) */
static int slow_B810(int i) {
    if (B(0xC34E)) return 0;
    int16_t v = (int16_t)(B(REC(i, R_SPEED)) - 5);
    if (v < 0x14) v = 0x14;
    B(REC(i, R_SPEED)) = (uint8_t)v;
    if (i == 3) setW(0xC32E, 0xFFFF);
    return -1;
}
/* B84E(i): steer out (x + 2, at most 280) unless somebody is on the right */
static int right_B84E(int i) {
    if (B(0xC344) || B(0xC346)) return 0;
    int16_t v = (int16_t)(SW(LANE(i)) + 2);
    if (v > 0x118) v = 0x118;
    setW(LANE(i), (uint16_t)v);
    return -1;
}
/* B886(i): steer in (x - 2, at least 0) unless somebody is on the left */
static int left_B886(int i) {
    if (B(0xC34A) || B(0xC348)) return 0;
    int16_t v = (int16_t)(SW(LANE(i)) - 2);
    if (v < 0) v = 0;
    setW(LANE(i), (uint16_t)v);
    return -1;
}
/* D53E(j, n): chariot strength -= n (0 at least, signed); at 0 the wreck (+E = 11) unless already wrecked */
static void chariot_D53E(int j, uint8_t n) {
    if (SB(REC(j, R_BODY)) > (int8_t)n) B(REC(j, R_BODY)) = (uint8_t)(B(REC(j, R_BODY)) - n);
    else B(REC(j, R_BODY)) = 0;
    if (j == 3) setW(0xC36C, 0xFFFF);
    if (SB(REC(j, R_BODY)) <= 0 && B(REC(j, R_LAST)) != 0xB) B(REC(j, R_CMD)) = 0xB;
}
/* D590(j, n): horses -= n (0 at least, signed); speed = min(speed, top) */
static void horses_D590(int j, uint8_t n) {
    if (SB(REC(j, R_HORSE)) > (int8_t)n) B(REC(j, R_HORSE)) = (uint8_t)(B(REC(j, R_HORSE)) - n);
    else B(REC(j, R_HORSE)) = 0;
    if (j == 3) setW(0xC330, 0xFFFF);
    if (B(REC(j, R_TOP)) <= B(REC(j, R_SPEED))) B(REC(j, R_SPEED)) = B(REC(j, R_TOP));
}
/* B9C9(a, b, flag): two dice rng(100) < 87EA[type of a] (20 / 33 / 50); n hits: flag ? D53E(b, n) : D590(b, 1) */
static void hits_B9C9(int a, int b, int flag) {
    uint8_t n = 0;
    for (int si = 0; si < 2; si++) {
        uint16_t r = rng_roll(0x64);
        if (B(0x87EA + B(REC(a, R_TYPE))) > r) n++;
    }
    if (!n) return;
    if (flag) chariot_D53E(b, n);
    else horses_D590(b, 1);
}
/* B977(c, side): the entry of probe `side` (the chariot part preferred), B9C9(c, it, chariot?) */
static void bump_B977(int c, int side) {
    if (W(REC(c, R_RUN)) == 0) return;
    uint8_t e = B(0xC344 + 2 * side), e2 = B(0xC345 + 2 * side), v = e;
    if ((e2 & 0x20) && (e & 0x10)) v = e2;
    hits_B9C9(c, v & 0xF, v & 0x20);
}
/* B8BC(i, side): the deliberate lash (the horses preferred): the chariot there -1 strength; the horses there: they surge
   rng(10) (at least 20), their rest clock restarts, -1 horse strength (D590 also caps the speed at top). Bug 11. */
static void lash_B8BC(int i, int side) {
    if (W(REC(i, R_RUN)) == 0) return;
    uint8_t e = B(0xC344 + 2 * side), e2 = B(0xC345 + 2 * side), v = e;
    if ((e2 & 0x10) && (e & 0x20)) v = e2;
    if (v & 0x20) { chariot_D53E(v & 0xF, 1); return; }
    if (!(v & 0x10)) return;
    uint16_t j = g_original_bugs ? v : (uint16_t)(v & 0xF);          /* b914: 1990 kept the 10h flag in the index */
    uint16_t sp = (uint16_t)(0xC2C8 + 0x12 * j);                     /* [si + C2C8] */
    uint8_t before = B(sp);
    uint16_t r = rng_roll(0xA);
    B(sp) = (uint8_t)(B(sp) + (uint8_t)r);
    if (B(sp) < 0x14) B(sp) = 0x14;
    setW((uint16_t)(0xC324 + 2 * j), W(0xC322));
    horses_D590(v & 0xF, 1);
    if (B(sp) != before && (g_original_bugs ? v : j) == 3) setW(0xC32E, 0xFFFF);
}

/* ---------------------------------------------------------------- the race tick */
/* BAC5(i): screen y toward 100 + 8 d (d = the player's path point - racer i's, wrapped to -184..184), at most 4 px per tick
   while |d| <= 24 and the old and new y are not both past the same edge (-22 / 170); visible -22..170 */
static uint16_t screen_y_BAC5(int i) {
    int16_t d = (int16_t)(W(0xC316) - W(PREV(i)));
    if (d > 0) { if (d > 0xB8) d = (int16_t)(d - 0x170); }
    else if (d < -0xB8) d = (int16_t)(d + 0x170);
    int16_t t = (int16_t)(d * 8 + 0x64);                             /* shl di, 3; add di, 64h */
    int16_t ad = d < 0 ? (int16_t)-d : d;
    int jump = ad > 0x18;
    if (!jump) {
        if (SW(SCRY(i)) > 0xAA && t > 0xAA) jump = 1;
        else if (SW(SCRY(i)) < -0x16 && t < -0x16) jump = 1;
    }
    if (jump) setW(SCRY(i), (uint16_t)t);
    else {
        if (SW(SCRY(i)) > 0xAA) setW(SCRY(i), 0xAA);
        else if (SW(SCRY(i)) < -0x16) setW(SCRY(i), 0xFFEA);
        int16_t m = (int16_t)(t - SW(SCRY(i)));
        if (m < 0) m = (int16_t)-m;
        if (m > 4) m = 4;
        if (SW(SCRY(i)) > t) setW(SCRY(i), (uint16_t)(SW(SCRY(i)) - m));
        else if (SW(SCRY(i)) < t) setW(SCRY(i), (uint16_t)(SW(SCRY(i)) + m));
    }
    return (SW(SCRY(i)) <= 0xAA && SW(SCRY(i)) >= -0x16) ? 0xFFFF : 0;
}

static void crash_scene_D738(void);

/* B460(i, y_old): steering / reining every tick; at an animation boundary (the chariot node, or the crash node, at frame
   0) the speed capped at top and the pending command performed (switch B598); then the tree drawn and stepped */
static void step_B460(int i, int16_t y_old) {
    uint8_t c = B(REC(i, R_CMD));
    if (c == 0 || c == 9 || c == 10) {                               /* b492 */
        if (B(REC(i, R_CMD)) == 0xA) {
            if (!right_B84E(i) && B(0xC346)) bump_B977(i, 1);
        } else if (B(REC(i, R_CMD)) == 9) {
            if (!left_B886(i) && B(0xC348)) bump_B977(i, 2);
        }
        if (SW(SCRY(i)) > y_old && B(0xC34E)) { setW(SCRY(i), (uint16_t)y_old); bump_B977(i, 5); }    /* pushed back */
        if (SW(SCRY(i)) < y_old && B(0xC34C)) {                      /* b4f1: ran into the one in front */
            setW(SCRY(i), (uint16_t)y_old);
            if (g_original_bugs) bump_B977(B(0xC34C) & 0xF, 5);      /* bug 5 */
            else if (W(REC(B(0xC34C) & 0xF, R_RUN))) hits_B9C9(B(0xC34C) & 0xF, i, 0);   /* B977's running test kept */
        }
    } else if (c == 8) {                                             /* b50d: rein in */
        if (!slow_B810(i)) bump_B977(i, 5);
    }
    uint16_t ch = W(ROOT(i)), cr = W(ch);                            /* b51d */
    int at = (W(ch + N_ACT) && B(ch + N_FR) == 0) || (W(cr + N_ACT) && B(cr + N_FR) == 0);
    if (at) {                                                        /* b567 */
        if (B(REC(i, R_TOP)) < B(REC(i, R_SPEED))) {
            B(REC(i, R_SPEED)) = B(REC(i, R_TOP));
            if (i == 3) setW(0xC32E, 0xFFFF);
        }
        switch (B(REC(i, R_CMD))) {                                  /* b598: jump table B59D */
        case 0: case 8: case 9: case 10: pose_BA28(i, 0); break;
        case 2:                                                      /* b5be: lash left */
            if (B(0xC34A)) { lash_B8BC(i, 3); pose_BA28(i, 2); }
            else B(REC(i, R_CMD)) = 1;                               /* nobody there: a whip */
            break;
        case 3:                                                      /* b5dd: lash right */
            if (B(0xC344)) { lash_B8BC(i, 0); pose_BA28(i, 3); }
            else B(REC(i, R_CMD)) = 1;
            break;
        case 0xB:                                                    /* b5f1: the wreck: chariot off, crash on */
            setW(ch + N_ACT, 0);
            setW(cr + N_ACT, 0xFFFF);
            break;
        default: break;
        }
        if (B(REC(i, R_CMD)) == 1 && !B(0xC34C)) {                   /* b60c: the whip */
            if (B(REC(i, R_HORSE))) {
                accel_B7D2(i, (uint16_t)(rng_roll(0xA) + 0xA));
                B(REC(i, R_HORSE))--;
                if (i == 3) setW(0xC330, 0xFFFF);
            } else accel_B7D2(i, rng_roll(0xA));
            pose_BA28(i, 1);
            setW(REST(i), W(0xC322));
        }
        B(REC(i, R_LAST)) = B(REC(i, R_CMD));                        /* b665 */
        B(REC(i, R_CMD)) = 0;
        if (B(REC(i, R_LAST)) == 0xB) B(REC(i, R_CMD)) = 5;
        else if (B(REC(i, R_LAST)) == 5) {                           /* b686: the runaway team */
            uint16_t c2 = W(W(ROOT(i)));
            setW(c2 + N_ACT, 0);
            setW(W(c2) + N_ACT, 0xFFFF);
            B(REC(i, R_CMD)) = 5;
        }
    }
    tree_BD0A(ROOT(i), SW(LANE(i)), SW(SCRY(i)));                    /* b6ad */
}

/* B6CB(i): state 5 (crashing): the runaway team; off screen: out; 24 ticks of veering (speed by dice, steer by a coin); the
   player then waits until no opponent is on screen and gets the crash scene (D738, all racers out); bug 6 */
static void crash_B6CB(int i) {
    uint16_t cr = W(W(ROOT(i)));
    setW(cr + N_ACT, 0);
    setW(W(cr) + N_ACT, 0xFFFF);
    if (W(0xAA76) == 0) { B(REC(i, R_CMD)) = 7; B(REC(i, R_LAST)) = 7; setW(REC(i, R_RUN), 0); }
    B(REC(i, R_CRASH))++;
    if (B(REC(i, R_CRASH)) < 0x18) {
        uint8_t sp = g_original_bugs ? B(0xC2FE) : B(REC(i, R_SPEED));
        if (sp > 0x46) slow_B810(i);
        else accel_B7D2(i, (uint16_t)(rng_roll(0xA) + 0xA));
        B(REC(i, R_CMD)) = rng_roll(2) ? 0xA : 9;
        return;
    }
    if (i == 3) {
        if (W(0xAAB4) == 0) {
            for (uint16_t bx = 0xC2C2; bx < 0xC30A; bx = (uint16_t)(bx + 0x12)) setW(bx, 0);
            setW(W(W(W(W(0x83D2)))) + N_ACT, 0);                     /* the lying driver hidden */
            crash_scene_D738();
            return;
        }
        if ((B(REC(i, R_CRASH)) > 0x30 && B(REC(i, R_TOP)) < B(REC(i, R_SPEED))) || B(REC(i, R_SPEED)) > 0x46) slow_B810(i);
    } else if (B(REC(i, R_TOP)) / 3 < B(REC(i, R_SPEED))) slow_B810(i);
    B(REC(i, R_CMD)) = 0xA;
}

/* B190: one race tick: the view cleared, the speckles, each racer moved (blocked / pushed steps), laps, finish, screen y,
   crash, B460; the view and the minimap to the screen, the dots; the race ends when the player has finished or is out, or
   is the only one left */
static void tick_B190(void) {
    setW(0xAAB4, 0);
    setW(0xC322, (uint16_t)(W(0xC322) + 1));
    setW(0x9AFA, 7);
    if (nv_vga()) setW(0x9AFA, 0x11);
    nv_target_work_67E4();                                           /* [9B26] != 6 */
    nv_fill_7E74(0, 0x3D, 0x13F, 0xAA);
    for (int i = 0; i < 4; i++) setW(PREV(i), W(PATH(i)));           /* (mode 6 erases the old dots first) */
    memmove(&g_ds[0xC0DA], &g_ds[0x83D4], 8);
    speckles_B11C(-1);
    for (int i = 0; i < 4; i++) {                                    /* b229 */
        if (W(REC(i, R_RUN)) == 0) continue;
        sides_D5E1(i);
        uint8_t step = B(REC(i, R_STEP));
        if (B(0xC34C)) { uint8_t f = B(REC(B(0xC34C) & 0xF, R_STEP)); if (f <= step) step = f; }   /* blocked in front */
        if (B(0xC34E)) { uint8_t r = B(REC(B(0xC34E) & 0xF, R_STEP)); if (r >= step) step = r; }  /* pushed from behind */
        setW(REC(i, R_POS), (uint16_t)(W(REC(i, R_POS)) + step));
        setW(PATH(i), (uint16_t)(W(REC(i, R_POS)) / 0x4B));
        if (W(PATH(i)) >= 0x170) {
            setW(REC(i, R_POS), (uint16_t)(W(REC(i, R_POS)) - 0x6BD0));
            setW(PATH(i), (uint16_t)(W(PATH(i)) - 0x170));
            B(REC(i, R_LAPS))++;
            if (SB(REC(i, R_LAPS)) > SB(0xC350)) {
                setW(0xC332, 0xFFFF);
                B(0xC350)++;
                if (B(0xC350) == 2) {                                /* the leader's last lap: the finish line */
                    memmove(&g_ds[0xC0DA], &g_ds[0x83DC], 8);
                    setW(0x9AFA, 0xF);
                    rc_line_979C(0x78, 0x16, 0x78, 0x1C);
                    memmove(&g_ds[0xC0DA], &g_ds[0x83D4], 8);
                }
            }
        }
        if (B(REC(i, R_LAPS)) == 3 && B(REC(i, R_LAST)) != 7) {      /* b30f; bug 1 */
            B(REC(i, R_LAST)) = 7;
            if (g_original_bugs || W(FIN(i)) == 0xFFFF) setW(FIN(i), W(0xC322));
        }
        int16_t y_old = SW(SCRY(i));                                 /* si */
        setW(0xAA76, screen_y_BAC5(i));
        if (i != 3) {
            setW(0xAAB4, (uint16_t)(W(0xAAB4) | W(0xAA76)));
            if (W(0xAA76) == 0 && B(0xC301) == 5) {                  /* the player crashing: off-screen opponents leave */
                B(REC(i, R_CMD)) = 7; B(REC(i, R_LAST)) = 7; setW(REC(i, R_RUN), 0);
            }
        }
        if (B(REC(i, R_LAST)) == 5) crash_B6CB(i);
        if (W(REC(i, R_RUN))) step_B460(i, y_old);
    }
    memmove(&g_ds[0xC0DA], &g_ds[0x83DC], 8);
    nv_target_screen_680F();
    nv_restore_6973(SW(0x83D4), SW(0x83D6), SW(0x83D8), SW(0x83DA));
    nv_restore_6973(0x68, 0x16, 0x113, 0x27);
    int n = 0;
    for (int i = 0; i < 4; i++) {                                    /* b3eb: the dots */
        if (W(REC(i, R_RUN)) == 0) continue;
        if (g_original_bugs ? B(REC(i, R_LAPS)) == 3 : SB(REC(i, R_LAPS)) >= 3) continue;   /* bug 13 */
        n++;
        setW(0x9AFA, W(0x87DE + 2 * i));
        int x, y;
        rc_path_point(W(PATH(i)), &x, &y);
        rc_pixel_82DE(x, y);
        g_dot_y = (uint16_t)y;
    }
    if (W(0xC2F8) && B(0xC301) != 7) {
        if (n == 1 && B(0xC301) != 5 && B(0xC301) != 0xB) { setW(0xC32C, 0xFFFF); setW(0xC320, (uint16_t)(W(0xC322) + 1)); }
    } else setW(0xC32C, 0xFFFF);
}

/* D17A: for each racer still steering: on straights the skid fades (-3) and the step = speed; in the curve approach (zone A)
   and the curve (zone B) the skid moves toward 15 - (limit - speed) / 4; in zone B the step is reduced by the lane, an
   overspeed d wrecks with rng(15) < d, and a big skid grinds the chariot on the right (both ways) or drifts outward */
static void curves_D17A(void) {
    for (int i = 0; i < 4; i++) {
        uint16_t sk = REC(i, R_SKID);
        if (W(REC(i, R_RUN)) == 0 || B(REC(i, R_LAST)) == 0xB || B(REC(i, R_CMD)) == 0xB) continue;
        uint16_t p = W(PATH(i));
        int zA = p > 0x152 || (p > 0x8E && p < 0xAC);
        int zB = (p != 0 && p < 0x2A) || (p > 0xAC && p < 0xD5);
        if (!zB && !zA) {
            int16_t v = (int16_t)(SB(sk) - 3);
            if (v < 0) v = 0;
            B(sk) = (uint8_t)v;
            B(W(ROOT(i)) + N_DX) = (uint8_t)v;
            B(REC(i, R_STEP)) = B(REC(i, R_SPEED));
            continue;
        }
        int16_t x = SW(LANE(i));
        int a = x < 0 ? -x : x;
        a >>= 4;
        if (x < 0) a = -a;
        uint8_t lane = (uint8_t)a;                                   /* [bp-4] */
        uint8_t lim = (uint8_t)((uint8_t)(lane + 0xF) << 2);         /* [bp-7] */
        if (zB) {
            int16_t prod = (int16_t)(B(0x8AA0 + lane) * B(REC(i, R_SPEED)));
            int16_t div = (int16_t)((uint16_t)(0xFFF1 - lane) << 2);
            int16_t q = (int16_t)(prod / div);
            B(REC(i, R_STEP)) = (uint8_t)(q + B(REC(i, R_SPEED)));
        }
        int8_t d = (int8_t)(lim - B(REC(i, R_SPEED)));               /* [bp-1] */
        int8_t t;                                                    /* [bp-3] */
        if (d > 0) { int q = d / 4; if (q > 0xF) q = 0xF; t = (int8_t)(0xF - q); }
        else t = 0xF;
        int8_t s = SB(sk);
        if (t > s) { int16_t cv = (int16_t)(s + 3); B(sk) = (uint8_t)(cv <= t ? cv : t); }
        else if (t < s) { int16_t cv = (int16_t)(s - 3); B(sk) = (uint8_t)(cv >= t ? cv : t); }
        if (SB(sk) < 0) B(sk) = 0;
        if (d < 0) {                                                 /* d2aa */
            t = (int8_t)(uint8_t)(d < 0 ? -d : d);
            if (zB && (int8_t)rng_roll(0xF) < t) B(REC(i, R_CMD)) = 0xB;   /* overspeed in the curve: the wreck */
        }
        if (zB && t > 8) {                                           /* d2d1 */
            sides_D5E1(i);
            uint8_t e1 = B(0xC346), e2 = B(0xC347);
            if (B(REC(i, R_LAST)) != 5) {
                if (e1 & 0x20) { hits_B9C9(i, e1 & 0xF, -1); hits_B9C9(e1 & 0xF, i, -1); }
                else if (e2 & 0x20) { hits_B9C9(i, e2 & 0xF, -1); hits_B9C9(e2 & 0xF, i, -1); }
            }
            if (!e1 && !e2 && lane < 0xE) setW(LANE(i), (uint16_t)(SW(LANE(i)) + t / 8 + 1));   /* drift outward */
        }
        uint8_t m = B(sk);
        if ((int8_t)m > 0xF) m = 0xF;
        B(W(ROOT(i)) + N_DX) = m;
    }
}

/* DDBD(i) / DE10(i): the player beside us on the right / left: close in (steer, 25 ticks) beside our chariot, lash beside
   our horses; -1 when it applies */
static int attack_DDBD(int i) {
    if ((B(0xC346) & 0xF) == 3) { B(REC(i, R_CMD)) = 0xA; B(0x8BA8 + i) = 0xA; B(0x8BAC + i) = 0x19; return -1; }
    if ((B(0xC344) & 0xF) == 3) { B(REC(i, R_CMD)) = 3; B(0x8BA8 + i) = 0; B(0x8BAC + i) = 0x19; return -1; }
    return 0;
}
static int attack_DE10(int i) {
    if ((B(0xC348) & 0xF) == 3) { B(REC(i, R_CMD)) = 9; B(0x8BA8 + i) = 9; B(0x8BAC + i) = 0x19; return -1; }
    if ((B(0xC34A) & 0xF) == 3) { B(REC(i, R_CMD)) = 2; B(0x8BA8 + i) = 0; B(0x8BAC + i) = 0x19; return -1; }
    return 0;
}
static void steer(int i, uint8_t c) { B(REC(i, R_CMD)) = c; B(0x8BA8 + i) = c; B(0x8BAC + i) = 0x19; }   /* dbe6 */

/* DABA: the opponents' driving, per tick */
static void ai_DABA(void) {
    uint16_t lead;                                                   /* [bp-12h]: bug 10 */
    if (!g_original_bugs) lead = 0;
    else lead = B(0xC301) == 5 ? g_dot_y : 0xD499;
    for (int j = 0; j < 4; j++) {
        uint16_t v = (uint16_t)((int16_t)(SB(REC(j, R_LAPS)) + 1) * 0x170 + W(PREV(j)));
        if (v > lead) lead = v;
    }
    for (int i = 0; i < 3; i++) {
        if (W(REC(i, R_RUN)) == 0 || B(REC(i, R_LAST)) == 0xB || B(REC(i, R_LAST)) == 5) continue;
        int behind = 0;
        uint16_t p = W(PATH(i));
        int zA = p > 0x152 || (p > 0x8E && p < 0xAC);
        int zB = (p != 0 && p < 0x2A) || (p > 0xAC && p < 0xD5);
        uint16_t v = (uint16_t)((int16_t)(SB(REC(i, R_LAPS)) + 1) * 0x170 + W(PREV(i)));
        if (v < lead) {
            behind = -1;
            if (B(0xC301) == 5) { B(REC(i, R_CMD)) = 8; continue; }  /* the player crashing: rein in */
        }
        int16_t x = SW(LANE(i));
        int a = x < 0 ? -x : x;
        a >>= 4;
        if (x < 0) a = -a;
        if (a > 0xF) a = 0xF;
        uint8_t lane = (uint8_t)a;                                   /* [bp-1] */
        uint8_t base = (uint8_t)((uint8_t)(lane + 0xE) << 2);        /* [bp-5] */
        sides_D5E1(i);
        if (!zA && !zB) {                                            /* the straights */
            if (B(0x8BAC + i)) {
                if (--B(0x8BAC + i) == 0) B(0x8BA8 + i) = 0;
                B(REC(i, R_CMD)) = B(0x8BA8 + i);
                continue;
            }
            if (attack_DDBD(i) || attack_DE10(i)) continue;
            if (B(0xC34C)) { steer(i, lane < 7 ? 0xA : 9); continue; }
            if (!behind && !((uint8_t)(B(REC(i, R_TOP)) + 0xA) > B(REC(i, R_SPEED)))) {   /* dbfd: never (speed <= top) */
                uint16_t r = rng_roll(0xA);
                steer(i, r > 6 ? 0xA : r > 2 ? 9 : 0);
                continue;
            }
            if (lane > 0xA && rng_roll(2) == 0) { steer(i, 9); continue; }
            B(REC(i, R_CMD)) = 1;                                    /* the whip */
            B(0x8BA8 + i) = 0;
            continue;
        }
        int16_t r = (int16_t)(rng_roll(0x64) - B(0xAAC2 + i));       /* the curves: a speed cap */
        uint16_t aa = (uint16_t)(r < 0 ? -r : r);
        int clean = g_original_bugs ? r >= 0 : r < 0;                 /* bug 8 */
        int diff = B(0x048F);
        if (diff == 0 || diff == 1) {
            if (clean) B(0xAABE + i) = base;
            else {
                int k = B(0xC368 + i), cap = -1;
                int16_t e = 0;
                if (k <= 2) {
                    uint16_t rr = rng_roll(aa);
                    int16_t m = (int16_t)(k == 2 ? 3 * aa : aa);
                    int sh = diff == 0 ? (k == 1 ? 1 : 2) : (k == 1 ? 2 : 3);
                    if (k == 1 && diff == 0) e = (int16_t)(m / 2);
                    else { int am = m < 0 ? -m : m; am >>= sh; e = (int16_t)(m < 0 ? -am : am); }
                    cap = (uint8_t)(rr + base) - e;
                    if (!g_original_bugs && cap < 0x14) cap = 0x14;      /* bug 9 */
                    B(0xAABE + i) = (uint8_t)cap;
                }
            }
            if (diff == 1 && rng_roll(3) == 0) B(REC(i, R_CMD)) = 9;
        } else if (diff == 2) {
            B(0xAABE + i) = base;
            if (rng_roll(8) == 0) B(REC(i, R_CMD)) = 9;
        } else if (diff == 3) {
            B(0xAABE + i) = base;
            if (rng_roll(4) == 0) B(REC(i, R_CMD)) = 9;
        }
        if (B(0xAABE + i) < B(REC(i, R_SPEED))) B(REC(i, R_SPEED)) = B(0xAABE + i);   /* dd85 */
        if (B(REC(i, R_SPEED)) < 0x14) B(REC(i, R_SPEED)) = 0x14;
    }
}

/* CECC: the player's input -> C302 (rec 3 +E): 9 steer in, 10 steer out, 8 rein in; 1 whip, 2 / 3 lash left / right */
static void input_CECC(void) {
    int16_t si = 0;                                                  /* [bp-6] */
    uint8_t k = (uint8_t)key_read();                                 /* [bp-3] */
    key_flush();
    B(0xA2) = (uint8_t)screen_held_dirs();                           /* the INT 9 hook's arrow bits (port: sampled here) */
    if (W(0xC2F8) == 0 || B(0xC301) == 0xB || B(0xC302) == 0xB) return;
    if (W(0xBA)) {                                                   /* the mouse */
        B(0x8A9E) = 0;
        mouse_poll_input();
        int16_t di = (int16_t)(SW(0x8A9A) - SW(0xB6)), dv = (int16_t)(SW(0x8A9C) - SW(0xB8));
        if (SW(0xB6) < 0x14 || SW(0xB6) > 0x12C || SW(0xB8) < 0x14 || SW(0xB8) > 0xB4) {
            mouse_warp(0xA0, 0x64);
            setW(0x8A9A, 0xA0); setW(0x8A9C, 0x64);
        }
        int ady = dv < 0 ? -dv : dv, adx = di < 0 ? -di : di;
        if (ady >= adx) { if (ady > 2) { if (dv < 0) B(0x8A9E) |= 8; else if (dv > 0) B(0x8A9E) |= 4; } }
        else if (adx > 2) { if (di < 0) B(0x8A9E) |= 1; else if (di > 0) B(0x8A9E) |= 2; }
        if (W(0xB2)) {
            if ((B(0xB2) & 2) && (B(0xB2) & 1)) si = 1;
            else if (B(0xB2) & 2) si = 3;
            else { si = 0; if (B(0xB2) & 1) si = 2; }
        } else si = 0;
        setW(0x8A9A, W(0xB6)); setW(0x8A9C, W(0xB8));
    }
    if (!W(0xBA) && !k) B(0x8A9E) = 0;
    if (B(0xA2)) {
        if (B(0xA2) & 8) {
            B(0x8A9E) |= 4;
            if (B(0xC301) == 0) {
                if (B(0xA2) & 1) si = 3;
                else si = (B(0xA2) & 2) ? 2 : 1;
            }
        } else {
            if (B(0xA2) & 4) B(0x8A9E) |= 8;
            if (B(0xA2) & 2) B(0x8A9E) |= 2;
            else if (B(0xA2) & 1) B(0x8A9E) |= 1;
        }
    }
    key_flush();
    if (B(0x8A9E) & 2) B(0xC302) = 9;
    else if (B(0x8A9E) & 1) B(0xC302) = 0xA;
    else if (B(0x8A9E) & 8) B(0xC302) = 8;
    if (si) B(0xC302) = (uint8_t)si;
}

/* ---------------------------------------------------------------- the crash scene */
static void wait7(void) { while (nv_t_since0_01AE() < 7) nv_tick(); }
static void view_out(void) { nv_target_screen_680F(); nv_restore_6973(SW(0x83D4), SW(0x83D6), SW(0x83D8), SW(0x83DA)); }

/* D738: the player's crash: the empty chariot drives off, the stretcher bearers come diagonally, then one of two scenes
   (rng(2)): A the driver gets up and they leave with the empty stretcher, B he is carried off; Esc ends a phase */
static void crash_scene_D738(void) {
    int stop = 0;                                                    /* [bp-0Ah] */
    music_stop_5D20();
    uint16_t offs, list; int16_t nfr, base3; int vb;                 /* [bp-12h] [bp-0Eh] [bp-10h] [bp-20h] [bp-0Ch] */
    if (rng_roll(2)) { offs = 0x8B3A; list = 0x8B04; nfr = 0x1C; base3 = 0; vb = 0; }
    else { offs = 0x8B72; list = 0x8B20; nfr = 0x1A; base3 = 0x19; vb = -1; }
    int16_t x0 = SW(0xC340), ty = SW(0xC342);                       /* [bp-1Eh], [bp-1Ch] */
    int16_t bx = (int16_t)(x0 + 9), by = (int16_t)(ty + 0x1D);      /* [bp-1Ah], [bp-18h] */
    memmove(&g_ds[0xC0DA], &g_ds[0x83D4], 8);
    memmove(&g_ds[0xC1E4], &g_ds[0x83D4], 8);
    memmove(&g_ds[0x9AC0], &g_ds[0x83D4], 8);
    int16_t step = (int16_t)((int16_t)(B(0xC2FE) << 3) / 0x4B);    /* [bp-8] */
    int16_t n1 = (int16_t)((int16_t)(0xAA - by) / 2 + 0xF);         /* [bp-6] */
    int16_t sx = (int16_t)(bx - 2 * n1 - 0x13);                      /* [bp-16h] */
    int16_t sy = (int16_t)(by + 2 * (n1 + 6));                       /* [bp-14h] */
    key_flush();
    for (int16_t si = 0; n1 > si; si++) {                            /* phase 1 */
        nv_t_reset_00F8();
        nv_target_work_67E4();
        clear_DA2E();
        if (ty >= -0x16) { ty = (int16_t)(ty - step); tree_BD0A(W(0x83D2), x0, ty); }
        rc_sprite_8E62((uint16_t)(W(0xC352) + 0x196), bx + 0xE, by + 6);
        rc_sprite_8E62((uint16_t)(0xE * (si & 1) + W(0xC352)), sx, sy);
        sx = (int16_t)(sx + 2); sy = (int16_t)(sy - 2);
        if ((key_read() & 0xFF) == 0x1B || screen_quit()) { stop = -1; break; }
        wait7();
        view_out();
    }
    if (!stop) {                                                     /* phase 2 */
        uint16_t di = offs;
        for (int16_t si = 0; nfr > si; si++, di = (uint16_t)(di + 2)) {
            nv_t_reset_00F8();
            nv_target_work_67E4();
            clear_DA2E();
            if (si <= 7) rc_sprite_8E62((uint16_t)(W(0xC352) + 0x196), bx + 0xE, by + 6);
            if (!vb && si > 0x13) rc_sprite_8E62((uint16_t)(W(0xC352) + 0x196), bx - 9, by + 7);
            rc_sprite_8E62((uint16_t)(0xE * B(list + si) + W(0xC352)), bx + SB(di), by + SB(di + 1));
            if ((key_read() & 0xFF) == 0x1B || screen_quit()) { stop = -1; break; }
            wait7();
            view_out();
        }
    }
    sx = (int16_t)(sx + 0x1B); sy = (int16_t)(sy - 0x12);             /* d952 */
    if (vb) { sx = (int16_t)(sx + 9); sy = (int16_t)(sy - 2); }
    int16_t si = 0;
    int di = stop;
    while (!di) {                                                    /* phase 3 */
        if (sx >= 0x13F || sy <= 0x15) break;
        si++;
        nv_t_reset_00F8();
        nv_target_work_67E4();
        clear_DA2E();
        if (!vb) rc_sprite_8E62((uint16_t)(W(0xC352) + 0x196), bx - 9, by + 7);
        rc_sprite_8E62((uint16_t)(0xE * ((si & 1) + base3) + W(0xC352)), sx, sy);
        sx = (int16_t)(sx + 2); sy = (int16_t)(sy - 2);
        if ((key_read() & 0xFF) == 0x1B || screen_quit()) di = -1;
        wait7();
        view_out();
    }
    nv_target_screen_680F();
    clear_DA2E();
    memmove(&g_ds[0xC0DA], &g_ds[0x83DC], 8);
    memmove(&g_ds[0xC1E4], &g_ds[0x83DC], 8);
    memmove(&g_ds[0x9AC0], &g_ds[0x83DC], 8);
    key_flush();
}

/* ---------------------------------------------------------------- loading */
/* BC15: archive 0; sheets race, laps, med; the four trees numbered; 0 = failure */
static int sheets_BC15(void) {
    int r = rc_archive_0F7B();                                       /* [bp-6] */
    if (r) {
        setW(0xC2BC, rc_sheet_163B(dstr(0x83EC)));                   /* "race" */
        setW(0xC306, rc_sheet_163B(dstr(0x83F1)));                   /* "laps" */
        setW(0xC352, rc_sheet_163B(dstr(0x83F6)));                   /* "med" */
        /* 11A9 archive close */
    }
    if (W(0xC2BC) == 0 || W(0xC306) == 0) return 0;
    for (int si = 0; si < 8; si += 2) {
        uint8_t n = 0;                                               /* [bp-3] */
        number_BCA0(&n, W(0x83BC + si), W(0x83CC + si));
    }
    return r;
}
static void sheets_free_BCF4(void) { rc_sheet_19A0(W(0xC352)); rc_sheet_19A0(W(0xC306)); rc_sheet_19A0(W(0xC2BC)); }
/* BDA4: archive 0; raceest (picture slot 3), gallery1 (C356), sheet race2 (C354); C364 = all loaded */
static void gallery_BDA4(void) {
    setW(0xC364, (uint16_t)rc_archive_0F7B());
    if (!W(0xC364)) return;
    setW(0xC364, (uint16_t)rc_picture_13A8());
    if (W(0xC364)) {
        setW(0xC364, (uint16_t)rc_gallery1_1273());
        if (W(0xC364)) {
            setW(0xC354, rc_sheet_163B(dstr(0x8407)));              /* "race2" */
            if (!W(0xC354)) { rc_gallery1_free_86A0(); setW(0xC364, 0); }
        }
    }
    /* 11A9 archive close */
}
/* C2C2: gallery1 and race2 freed if loaded */
static void gallery_free_C2C2(void) {
    if (!W(0xC364)) return;
    rc_gallery1_free_86A0();
    rc_sheet_19A0(W(0xC354));
    setW(0xC364, 0);
}
/* C70F: the cancel */
static void cancel_C70F(void) {
    sheets_free_BCF4();
    gallery_free_C2C2();
    rc_circus_free();
    music_unload_5E1A(2);
    music_unload_5E1A(3);
}

/* ---------------------------------------------------------------- parades */
static int any_input(void) { return (key_read() & 0xFF) || W(0xB2) || screen_quit(); }   /* 03AA / [B2]; port: the end */

/* BE10: the pre-race parade: one chariot (plain, whatever was bought) drives in from x 331 by 6 px a frame; at x <= 56 it
   stands and prances (rng(25) a frame) until a key or a button; a key during the drive skips the standing */
static void parade_BE10(void) {
    int si = 0, stop = 0;                                            /* [bp-6] */
    int16_t x = 0x14B;                                               /* [bp-4] */
    if (W(0xC364) == 0) goto out;
    music_play_5D29(2);
    key_flush();
    setW(0x9AFA, 0);
    nv_fill_7E74(0, 0, 0x13F, 0xC7);
    rc_sprite_8E62(RC_GALLERY1, 0, 0);
    rc_blit_8790();
    nv_save_691B(0, 0, 0x13F, 0xC7);
    nv_flip_6887();
    wait_release();
    while (!stop) {                                                  /* be77 */
        nv_t_reset_00F8();
        mouse_poll_input();
        rc_blit_8790();
        rc_brazier_C299();
        x = (int16_t)(x - 6);
        rc_chariot_C23D(0, x, si);
        if (x <= 0x38) goto stand;                                   /* bea2 */
        if (++si >= 4) si = 0;
        wait7();
        nv_flip_6887();
        if (any_input()) stop = -1;
    }
stand:                                                               /* bed3 */
    si = 0x3E;
    int f = 0;                                                       /* [bp-2] */
    while (!stop) {
        nv_t_reset_00F8();
        mouse_poll_input();
        rc_blit_8790();
        rc_brazier_C299();
        rc_chariot_C23D(0, si - 8, f + 4);
        wait7();
        nv_flip_6887();
        if (rng_roll(0x19) == 0) {
            f = (f + 1) & 1;
            if (si <= 0x3E) si++; else si--;
        }
        if (any_input()) stop = -1;
    }
    nv_page_6837();
out:                                                                 /* bf4f */
    music_stop_5D20();
    music_unload_5E1A(2);
    key_flush();
    if (W(0xB2)) wait_release();
}

/* BF6A(mode): mode 0 the winner's parade (the 4 racers, song 2), mode 1 the triumph (10 chariots of random types, song 0);
   the last chariot (the player, plain) stops at x 56 and raises his arms (race2 16..19; mode 1: the figure in red, 20..24),
   rng(15) a frame on the first 34 frames; a key or a button ends a phase; at most 76 frames of the last stage */
static void parade_BF6A(int mode) {
    int di = 0;                                                      /* [bp-0Ah] / di */
    uint8_t w = 3;                                                   /* [bp-5] */
    int n = 4;                                                       /* [bp-8] */
    uint8_t type[10] = { 0 };
    int16_t x[10] = { 0 }, on[10] = { 0 }, f[10] = { 0 };
    if (W(0xC364) == 0) goto out;
    setW(0x9AFA, 0);
    nv_fill_7E74(0, 0, 0x13F, 0xC7);
    if (mode == 1) {
        n = 10; w = 9;
        music_play_5D29(0);
        for (int i = 0; i < 10; i++) type[i] = (uint8_t)rng_roll(3);
    } else {
        music_play_5D29(2);
        int i = 0;
        for (uint16_t a = 0xC2C6; a < 0xC30E; a = (uint16_t)(a + 0x12)) type[i++] = B(a);
    }
    for (int i = 0; i < n; i++) { x[i] = (int16_t)(0x14B + 0x5A * i); on[i] = -1; f[i] = 0; }
    x[w] = (int16_t)(x[w] + 0x2D);
    type[w] = 0;
    key_flush();
    rc_sprite_8E62(RC_GALLERY1, 0, 0);
    rc_blit_8790();
    nv_save_691B(0, 0, 0x13F, 0xC7);
    nv_flip_6887();
    wait_release();
    while (!di) {                                                    /* phase 1, c05c */
        nv_t_reset_00F8();
        mouse_poll_input();
        rc_blit_8790();
        rc_brazier_C299();
        for (int i = 0; i < n; i++) {
            if (!on[i]) continue;
            x[i] = (int16_t)(x[i] - 6);
            rc_chariot_C23D(type[i], x[i], f[i]);
            if (mode == 1 && i == w) rc_sprite_8E62((uint16_t)(W(0xC354) + 0x118), x[i] + 0x32, 0x8E);
            if (x[i] <= -0x5A) on[i] = 0;
            if (++f[i] >= 4) f[i] = 0;
        }
        wait7();
        nv_flip_6887();
        if (any_input()) di = -1;
        if (x[w] <= 0x38) break;
    }
    int8_t cnt = 0, stage = 0;                                       /* [bp-2], [bp-1] */
    f[w] = 0;
    while (!di) {                                                    /* phase 2, c128 */
        nv_t_reset_00F8();
        mouse_poll_input();
        rc_blit_8790();
        rc_brazier_C299();
        if (stage < 4) {
            rc_chariot_C23D(stage + 0xB, x[w], f[w] + 4);
            if (mode == 1) rc_sprite_8E62((uint16_t)(0xE * stage + W(0xC354) + 0x126), x[w] + 0x32, 0x8E);
        } else {
            rc_chariot_C23D(0, x[w], f[w] + 4);
            if (mode == 1) rc_sprite_8E62((uint16_t)(W(0xC354) + 0x118), x[w] + 0x32, 0x8E);
        }
        wait7();
        nv_flip_6887();
        cnt++;
        if (SB(0x8412 + stage) < cnt) stage++;                       /* c1bd: the DS bytes after the 5 thresholds too */
        if (stage <= 4 && rng_roll(0xF) == 0) {
            f[w] = (int16_t)((f[w] + 1) & 1);
            if (x[w] <= 0x38) x[w]++; else x[w]--;
        }
        if (any_input() || cnt > 0x4B) di = -1;
    }
out:                                                                 /* c216 */
    if (mode != 1) { music_stop_5D20(); music_unload_5E1A(2); }
    key_flush();
    if (W(0xB2)) wait_release();
    nv_page_6837();
}

/* ---------------------------------------------------------------- pre-race */
/* DA5A(i): province rng(27), type rng(3), strategy rng(4) (3 -> Steady), skill rng(11) + B(8BA6) - 5 */
static void field_DA5A(int i) {
    B(REC(i, R_PROV)) = (uint8_t)rng_roll(0x1B);
    B(REC(i, R_TYPE)) = (uint8_t)rng_roll(3);
    uint8_t s = (uint8_t)rng_roll(4);
    if (s == 3) s = 1;
    B(0xC368 + i) = s;
    B(0xAAC2 + i) = (uint8_t)(rng_roll(0xB) + B(0x8BA6) - 5);
}
/* CBE6: the 368-point path from the polyline DS:896A..8A89 ((-1, -1) = pen up: two (-1, -1) points); all racers at 220 */
static void path_CBE6(void) {
    rc_path_alloc();
    uint16_t n = 0;                                                  /* [bp-6] */
    int16_t px = (int16_t)(SW(0x896A) + 1), py = SW(0x896C);         /* [bp-8], [bp-0Ch] */
    for (uint16_t bx = 0x896A; bx < 0x8A8A; bx = (uint16_t)(bx + 4)) {
        int16_t X = SW(bx), Y = SW(bx + 2), nx, ny;
        if (X == -1) {
            rc_path_set(n, -1, -1); n++;
            rc_path_set(n, -1, -1); n++;
            nx = SW(bx + 4); ny = SW(bx + 6);
        } else {
            int ddx = px - X, ddy = py - Y;
            if (ddx < 0) ddx = -ddx;
            if (ddy < 0) ddy = -ddy;
            if (ddx > 1) {
                if (X < px) { for (int16_t c = px; c >= X; c--) { rc_path_set(n, c, Y); n++; } }
                else { for (int16_t c = px; X >= c; c++) { rc_path_set(n, c, Y); n++; } }
            } else if (ddy > 1) {
                if (Y < py) { for (int16_t c = py; !(Y > c); c--) { rc_path_set(n, X, c); n++; } }
                else { for (int16_t c = py; !(Y < c); c++) { rc_path_set(n, X, c); n++; } }
            } else { rc_path_set(n, X, Y); n++; }
            nx = X; ny = Y;
        }
        py = ny; px = nx;
    }
    for (int i = 0; i < 4; i++) { setW(PREV(i), 0xDC); setW(PATH(i), 0xDC); }
}

/* C730: "What Skulduggery?" (the costs against the treasury minus the bet: [634A] holds AABC meanwhile) */
static void skulduggery_C730(void) {
    uint16_t saved = W(0x634A), spent = 0;                           /* [bp-4], si */
    int done = 0;                                                    /* [bp-2] */
    setW(0x634A, W(0xAABC));
    menu_draw_5614(0x891C, 1);
    while (!done) {
        province_panel3512();
        province_rome_panel6f0c(0);
        uint16_t di = menu_run_54C8(0x891C);
        if (di == 0xFF) done = -1;
        else switch (di) {
        case 0:                                                      /* Bribe Opponent: 10 */
            if (W(0x634A) < 0xA) msg_59D4(W(0x880A));
            else if (B(0xAABA) >= 3) msg_59D4(0x8930);               /* "All Opponents Bribed" */
            else {
                setW(0x634A, (uint16_t)(W(0x634A) - 0xA));
                spent = (uint16_t)(spent + 0xA);
                ds_cpy(0xAC60, dstr(W(0x8818 + 2 * B(0xAABA))));     /* "Red" / "Green" / "Blue" */
                ds_cat(0xAC60, dstr(W(0x881E)));                     /* " driver has been paid\n10 talents to let you win." */
                uint16_t r = rng_roll(0xA);
                B(REC(B(0xAABA), R_TOP)) = (uint8_t)(B(REC(B(0xAABA), R_TOP)) - (uint8_t)r);
                B(0xAABA)++;
                msg_59D4(0xAC60);
            }
            break;
        case 1:                                                      /* Hire Physician: 30 */
            if (W(0x634A) < 0x1E) msg_59D4(W(0x880A));
            else if (W(0xAAB6)) msg_59D4(W(0x8824));                 /* "You can only do that once" */
            else {
                setW(0xAAB6, 0xFFFF);
                setW(0x634A, (uint16_t)(W(0x634A) - 0x1E));
                spent = (uint16_t)(spent + 0x1E);
                msg_59D4(W(0x8820));
                B(0xC2FD) = (uint8_t)(B(0xC2FD) + rng_roll(0xA));
            }
            break;
        case 2:                                                      /* Invoke Gods: 50 */
            if (W(0x634A) < 0x32) msg_59D4(W(0x880A));
            else if (W(0xAAB8)) msg_59D4(W(0x8824));
            else {
                setW(0xAAB8, 0xFFFF);
                setW(0x634A, (uint16_t)(W(0x634A) - 0x32));
                spent = (uint16_t)(spent + 0x32);
                msg_59D4(W(0x8822));
                B(0xC304) = (uint8_t)(B(0xC304) + rng_roll(0xA) + 0xA);
            }
            break;
        case 3: done = -1; break;
        default: break;
        }
        menu_mark_585B(0x891C, di, 0);
    }
    setW(0xAABC, W(0x634A));
    setW(0x634A, (uint16_t)(saved - spent));
}

/* C88E: "What Will You Bet?": "Your Bet: <n> Talents\nOdds: <odds> to 1" at (78, 150); +10 / -10 / the limit = min(treasury,
   100) */
static void bet_C88E(void) {
    uint16_t di = 0;
    int done = 0;
    menu_draw_5614(0x890A, 1);
    while (!done) {
        setW(0xAABC, (uint16_t)(W(0x634A) - W(0xC366)));
        province_treasury44e5(W(0xAABC));                            /* leaves [9AFA] = [6346], [9B69] = 0 */
        char b[0x52];
        snprintf(b, sizeof(b), "%s%d%s%d%s", dstr(0x8945), SW(0xC366), dstr(0x8950), (int)B(0x87EE + B(0xC2FC)), dstr(0x8960));
        nv_fill_7E74(0xA, 0x96, 0x91, 0xB4);
        nv_text_8BE4(b, 0x4E, 0x96);
        uint16_t si = W(0x634A);
        if (si > 0x64) si = 0x64;
        di = menu_run_54C8(0x890A);
        switch (di) {
        case 0: setW(0xC366, (uint16_t)add_cap(SW(0xC366), 0xA, (int16_t)si)); break;
        case 1: setW(0xC366, (uint16_t)sub_050F(SW(0xC366), 0xA, 0)); break;
        case 2: setW(0xC366, si); break;
        case 3: case 0xFF: done = -1; break;
        default: break;
        }
        menu_mark_585B(0x890A, di, 0);
    }
}

/* C2DE: -1 = race, 0 = cancelled */
static int prerace_C2DE(void) {
    int bought = 0;                                                  /* [bp-4] */
    setW(0xC322, 0);
    if (!sheets_BC15()) return 0;
    gallery_BDA4();
    int si = rc_archive_0F7B();
    if (si) {
        si = rc_circus_load();
        if (si) rc_circus_draw();
        /* 11A9 archive close */
    }
    if (!si) { sheets_free_BCF4(); gallery_free_C2C2(); return 0; }
    music_load_5D73(3);
    music_load_5D73(2);
    uint16_t bg = nv_vga() ? 0x11 : 7;                               /* [bp-0Ah] */
    setW(0x9AFA, bg);
    nv_fill_7E74(0, 0x3D, 0x13F, 0xC7);
    for (int i = 0; i < 4; i++) {                                    /* c382: the field once per turn */
        if (W(0x892E) != W(0x634C)) field_DA5A(i);
        setW(REC(i, R_RUN), 0xFFFF);
        setW(REST(i), W(0xC322));
    }
    setW(0x892E, W(0x634C));
    for (int i = 0; i < 3; i++) rc_card_CDCE(i);
    B(0xC2FB) = 0;                                                   /* the player's home: Italia */
    menu_draw_5614(0x88E6, 2);
    while (!bought) {                                                /* c3ce: "Race what type of Chariot?" */
        uint16_t s = menu_run_54C8(0x88E6);
        menu_mark_585B(0x88E6, s, 0);
        if (s <= 2) {
            if (B(0x87F2 + s) > W(0x634A)) msg_59D4(W(0x880A));      /* "You don't have enough Talents" */
            else {
                B(0xC2FC) = (uint8_t)s;
                setW(0x634A, (uint16_t)(W(0x634A) - B(0x87F2 + s)));
                bought = -1;
                setW(0x9AFA, bg);
                nv_fill_7E74(6, 0x59, 0x9D, 0xC7);
                rc_card_CDCE(3);
                msg_59D4(0x3E02);                                    /* "": a bare "Click to continue" over your card */
            }
        } else if (yes_no(dstr(W(0x8814))) || screen_quit()) {       /* "Are You Sure\nYou Don't Want To Race?"; port: the end */
            cancel_C70F();
            return 0;
        }
    }
    rc_circus_draw();                                                /* c44d */
    setW(0x9AFA, bg);
    nv_fill_7E74(0, 0x3D, 0x13F, 0xC7);
    uint16_t r = rng_roll(4);                                        /* [bp-8]: the start lanes */
    for (int i = 0; i < 4; i++) {                                    /* c490 */
        uint8_t t = B(REC(i, R_TYPE));
        tree_type_BBAD(t, i);
        B(REC(i, R_TOP)) = 0x64;
        if (i != 3) B(REC(i, R_TOP)) = 0x6E;
        B(REC(i, R_HORSE)) = B(0x87E6 + t);
        B(REC(i, R_SPEED)) = B(REC(i, R_STEP)) = (uint8_t)(B(REC(i, R_TOP)) / 3);
        B(REC(i, R_SKID)) = 0;
        B(REC(i, R_BODY)) = B(0x87FA + t);
        B(REC(i, R_LAPS)) = B(0xC350);
        B(REC(i, R_LAST)) = 0; B(REC(i, R_CMD)) = 0;
        setW(REC(i, R_POS), 0x4074);
        B(REC(i, R_CRASH)) = 0;
        setW(FIN(i), 0xFFFF);
        setW(LANE(i), (uint16_t)((uint8_t)((uint8_t)r + (uint8_t)i) & 3) * 0x46);
        setW(SCRY(i), 0x64);
    }
    setW(0xAABC, W(0x634A));
    B(0xAABA) = 0;
    setW(0xC366, 0); setW(0xAAB8, 0); setW(0xAAB6, 0);
    int go = 0;                                                      /* si */
    while (!go) {                                                    /* c530: "How Shall We Prepare?" */
        setW(0xAABC, (uint16_t)(W(0x634A) - W(0xC366)));
        province_panel3512();
        province_rome_panel6f0c(0);
        province_treasury44e5(W(0xAABC));
        menu_draw_5614(0x88F8, 1);
        uint16_t s = menu_run_54C8(0x88F8);
        if (s == 0) {                                                /* Race Now */
            if (W(0xC366) == 0 && W(0xAABC) != 0) go = yes_no(dstr(0x87BA)) ? -1 : 0;   /* "You Have Not Bet\nRace Anyway?" */
            else go = -1;
        } else if (s == 1) skulduggery_C730();
        else if (s == 2) bet_C88E();
        else if (s == 3 || s == 0xFF) {                              /* Exit */
            if (yes_no(dstr(W(0x8814)))) { cancel_C70F(); return 0; }
        }
        if (!go && screen_quit()) { cancel_C70F(); return 0; }       /* port: the input ended */
    }
    setW(0x634A, (uint16_t)(W(0x634A) - W(0xC366)));                 /* c5bd: the bet is paid */
    parade_BE10();
    rc_circus_draw();                                                /* c5cc: the race screen */
    setW(0x9AFA, bg);
    nv_fill_7E74(0, 0x3D, 0x13F, 0xC7);
    ui_box(nv_fb, 1, 0xAB, 0x13E, 0xC6, nv_vga() ? 0 : SW(0x6346));  /* the dashboard */
    gl_box_3233(0xB, 0xB4, 0x7F, 0xBD, SW(0x6346));
    gl_box_3233(0xC4, 0xB0, 0x138, 0xB8, SW(0x6346));
    gl_box_3233(0xC4, 0xB9, 0x138, 0xC1, SW(0x6346));
    nv_font(0xA3B4);                                                 /* the small font */
    setW(0x9B69, 0xF);
    rc_text_899C(dstr(0x87D8), 7, 0xB5);                             /* "S" */
    rc_text_899C(dstr(0x87DA), 0xBF, 0xB1);                          /* "H" */
    rc_text_899C(dstr(0x87DC), 0xBF, 0xBA);                          /* "C" */
    nv_font(0xA05C);
    rc_speed_D400();
    rc_body_D4EC();
    rc_horses_D49A();
    rc_laps_D392();
    nv_save_691B(0, 0, 0x13F, 0xC7);
    mouse_warp(0xA0, 0x64);
    rc_circus_free();
    path_CBE6();
    nv_target_work_67E4();
    setW(0x9AFA, bg);
    nv_fill_7E74(0, 0x3D, 0x13F, 0xAA);
    nv_target_screen_680F();
    setW(0xC102, (uint16_t)(W(0xC102) + 1));                         /* races entered */
    setW(0xC100, (uint16_t)(W(0xC100) + 1));                         /* games sponsored */
    return -1;
}

/* ---------------------------------------------------------------- the result */
/* C9B6: the order (finish tick, then the position; bugs 2 and 3), the message, score, winnings, odds, the field's skill
   base by place and difficulty; the winning opponent's province gets -10 unrest */
static void result_C9B6(void) {
    uint16_t o[4] = { 0, 1, 2, 3 };
    for (int a = 0; a < 3; a++)
        for (int b = 3; b >= a + 1; b--) {
            uint16_t A = o[a], Bb = o[b];
            int sw = 0;
            if (W(FIN(A)) > W(FIN(Bb))) sw = 1;
            else if (W(FIN(Bb)) == W(FIN(A))) {
                if (!g_original_bugs && SB(REC(Bb, R_LAPS)) != SB(REC(A, R_LAPS))) sw = SB(REC(Bb, R_LAPS)) > SB(REC(A, R_LAPS));
                else {
                    uint16_t pa = W(REC(A, R_POS)), pb = W(REC(Bb, R_POS));
                    if (pb > pa) sw = 1;
                    else if (pa == pb && rng_roll(g_original_bugs ? 1 : 2) == 0) sw = 1;
                }
            }
            if (sw) { o[a] = Bb; o[b] = A; }
        }
    int p = 0;
    for (int k = 0; k < 4; k++) if (o[k] == 3) p = k;
    uint8_t t = B(0xC2FC);
    if (W(0xC320) == 0xFFFF) {                                       /* cb9c */
        msg_59D4(W(0x8816));                                         /* "You Failed To Finish" */
        if (B(0x87EE + t) < 5) B(0x87EE + t)++;
    } else {
        msg_59D4(W(0x880C + 2 * p));
        switch (p) {
        case 0: {                                                    /* "You Win The Race!" */
            score_add(0xA);
            B(0x634E) = (uint8_t)add_cap(B(0x634E), 1, 0x64);
            uint8_t od = B(0x87EE + t);
            setW(0x634A, (uint16_t)(W(0x634A) + (uint16_t)((od + 1) * W(0xC366))));
            int16_t g = (int16_t)(uint16_t)((od + 1) * W(0xC366));
            uint32_t tot = ((uint32_t)W(0xC098) << 16 | W(0xC096)) + (uint32_t)(int32_t)g;
            setW(0xC096, (uint16_t)tot); setW(0xC098, (uint16_t)(tot >> 16));
            if (od > 1) B(0x87EE + t)--;
            setW(0xC104, (uint16_t)(W(0xC104) + 1));
            parade_BF6A(0);
            break;
        }
        case 1: score_add(5); break;                                 /* "A Creditable Second" */
        case 2: score_add(1);                                        /* "A Poor Showing": falls into the 4th place */
            /* fall through */
        case 3: if (B(0x87EE + t) < 5) B(0x87EE + t)++; break;       /* "Utter, Ignominious Failure" */
        }
        B(0x8BA6) = (uint8_t)(B(0x8BA6) + B(0x8966 + p));
        switch (B(0x048F)) {
        case 0: B(0x8BA6) = (uint8_t)(B(0x8BA6) - 3); break;
        case 1: B(0x8BA6) = (uint8_t)(B(0x8BA6) + 3); break;
        case 2: B(0x8BA6) = (uint8_t)(B(0x8BA6) + 6); break;
        default: break;
        }
        int8_t v = SB(0x8BA6);
        if (v > 0x5F) v = 0x5F;
        if (v < 0x4B) v = 0x4B;
        B(0x8BA6) = (uint8_t)v;
    }
    if (o[0] != 3) {
        uint16_t pr = (uint16_t)(B(REC(o[0], R_PROV)) << 5);
        int v = SB(0x527E + pr) - 0xA;
        if (v < 1) v = 1;
        B(0x527E + pr) = (uint8_t)v;
    }
}

/* ---------------------------------------------------------------- D08A, the race */
void race_d08a(void) {
    /* the port's start-up state, as naval.c / glad.c: the work buffer's descriptor and the screen as the target */
    if (W(0xC1DE) == 0) { memmove(&g_ds[0xC1DA], &g_ds[0x9AB6], 18); setW(0xC1DE, 0x2687); }
    nv_target_screen_680F();
    g_input_gone = 0;
    B(0xC350) = 0xFF;                                                /* the leader's lap: -1 */
    speckles_B0E0();
    if (!prerace_C2DE()) return;
    setW(0x154E, W(0x634C));                                         /* d09e: one race per turn */
    setW(0xC32E, 0); setW(0xC36C, 0); setW(0xC330, 0); setW(0xC332, 0); setW(0xC32C, 0);
    music_play_5D29(3);
    nv_t_reset_00F8();
    nv_wait_011F(0x12);
    while (!W(0xC32C)) {                                             /* d0c7 */
        if (screen_quit()) { g_input_gone = 1; break; }              /* port: the input ended */
        nv_t_reset_00F8();
        ai_DABA();
        input_CECC();
        curves_D17A();
        tick_B190();
        for (int i = 0; i < 4; i++) {                                /* d0de: the horses rest */
            if ((uint16_t)(W(REST(i)) + 0x96) > W(0xC322)) continue;
            uint8_t mx = B(0x87E6 + B(REC(i, R_TYPE)));
            if (g_original_bugs || SB(REC(i, R_HORSE)) < (int8_t)mx)  /* bug 7 */
                B(REC(i, R_HORSE)) = (uint8_t)add_cap(SB(REC(i, R_HORSE)), 1, mx);
            setW(REST(i), W(0xC322));
            if (i == 3) setW(0xC330, 0xFFFF);
        }
        wait7();                                                     /* d12b */
        rc_gauges_D3C2(-1);
    }
    sheets_free_BCF4();
    rc_path_free();                                                  /* 74B7 */
    music_stop_5D20();
    music_unload_5E1A(2);
    music_unload_5E1A(3);
    if (!g_input_gone) result_C9B6();
    gallery_free_C2C2();
}

/* ---------------------------------------------------------------- DF58, the victory */
/* the box of DF74: 8C9F(s, minw 0, lines 1, 159, 99) grown by 7, 691B, 3134 in [6346], the text in colour 0 from Y0 + 7,
   the click line in 15 at Y1 - (height + gap) - 5, 35F9, 6973 */
static void stats_box(int lh) {
    const char *s = dstr(0xAC60);
    ui_box_t r = ui_text_rect(s, 0, 1, 0x9F, 0x63);
    r.x0 -= 7; r.y0 -= 7; r.x1 += 7; r.y1 += 7;
    ui_copy_rect(g_wrk, g_scr, r);
    ui_box(g_scr, r.x0, r.y0, r.x1, r.y1, SW(0x6346));
    setW(0x9B69, 0);
    int cx = (int16_t)(r.x1 + r.x0) / 2;
    ui_text_block(g_scr, cx, r.y0 + 7, s, 0);
    setW(0x9B69, 0xF);
    ui_text_block(g_scr, cx, r.y1 - lh - 5, ui_click_text(), 0xF);
    if (!screen_headless()) screen_present();
    wait_click();
    ui_copy_rect(g_scr, g_wrk, r);
    if (!screen_headless()) screen_present();
}
static long l32(uint16_t lo) { return (long)(int32_t)((uint32_t)W(lo) | (uint32_t)W(lo + 2) << 16); }

/* DF74: "THE MAKING OF A CONQUEROR" (bug 12: the singulars), then the epilogue DS:8BD8 */
static void stats_DF74(void) {
    int lh = B(0x9B5C) + B(0x9B5D);                                  /* [bp-4] */
    int fix = !g_original_bugs;
    ds_cpy(0xAC60, dstr(0x8CD0));
    if (W(0xC106)) {
        ds_cat(0xAC60, dstr(0x8D03));                                /* "successfully negotiated " */
        ds_cat_num(0xAC60, SW(0xC106));
        ds_cat(0xAC60, dstr(0x8D1C));                                /* " alliance" */
        if (W(0xC106) != 1) ds_cat(0xAC60, dstr(0x8D26));            /* "s" */
        ds_cat(0xAC60, dstr(0x8D28));                                /* ",\n" */
    }
    ds_cat(0xAC60, dstr(0x8D2B));                                    /* "won " */
    ds_cat_num(0xAC60, SW(0xC0FA));
    ds_cat(0xAC60, fix && W(0xC0FA) == 1 ? " battle and lost " : dstr(0x8D30));
    long lost = (long)SW(0xC0F8) - SW(0xC0FA);
    ds_cat_num(0xAC60, lost);
    ds_cat(0xAC60, fix && lost == 1 ? " battle,\nin which " : dstr(0x8D43));
    ds_cat_num(0xAC60, l32(0xC086));
    ds_cat(0xAC60, fix && l32(0xC086) == 1 ? " legionnaire was killed and "
                                           : dstr(0x8D57));          /* e088: " legionnaires were killed and " */
    ds_cat_num(0xAC60, l32(0xC082));
    ds_cat(0xAC60, fix && l32(0xC082) == 1 ? " enemy soldier\nwas slain.  "
                                           : dstr(0x8D76));          /* e0b5: " enemy soldiers\nwere slain.  " */
    if (W(0xC0FC)) {
        ds_cat(0xAC60, dstr(0x8D94));                                /* "You " */
        if (W(0xC0FE)) {
            ds_cat(0xAC60, dstr(0x8D99));                            /* "won " */
            ds_cat_num(0xAC60, SW(0xC0FE));
            ds_cat(0xAC60, fix && W(0xC0FE) == 1 ? " fleet battle\nand " : dstr(0x8D9E));
        }
        ds_cat(0xAC60, dstr(0x8DB2));                                /* "lost " */
        if ((uint16_t)(W(0xC0FE) - W(0xC0FC) + 1) == 0) ds_cat(0xAC60, dstr(0x8DB8));   /* "one fleet battle" */
        else {
            if (W(0xC0FC) == W(0xC0FE)) ds_cat(0xAC60, dstr(0x8DC9));                   /* "no" */
            else ds_cat_num(0xAC60, (long)SW(0xC0FC) - SW(0xC0FE));
            ds_cat(0xAC60, dstr(0x8DCC));                            /* " fleet battles" */
        }
        ds_cat(0xAC60, dstr(0x8DDB));                                /* ", in which " */
        ds_cat_num(0xAC60, l32(0xC08E));
        ds_cat(0xAC60, fix && l32(0xC08E) == 1 ? "\nof your ships was sunk and "
                                               : dstr(0x8DE7));      /* e1a6: "\nof your ships were sunk and " */
        ds_cat_num(0xAC60, l32(0xC08A));
        ds_cat(0xAC60, fix && l32(0xC08A) == 1 ? " enemy ship was sunk.\n\n"
                                               : dstr(0x8E05));      /* e1d3: " enemy ships were sunk.\n\n" */
    }
    if (W(0xC100)) {
        ds_cat(0xAC60, dstr(0x8E1F));                                /* "You sponsored " */
        ds_cat_num(0xAC60, SW(0xC100));
        ds_cat(0xAC60, fix && W(0xC100) == 1 ? " game, entered " : dstr(0x8E2E));
        if (!W(0xC102)) ds_cat(0xAC60, dstr(0x8E3F));                /* "no" */
        else ds_cat_num(0xAC60, SW(0xC102));
        ds_cat(0xAC60, fix && W(0xC102) == 1 ? " race,\nwinning " : dstr(0x8E42));
        if (!W(0xC104)) ds_cat(0xAC60, dstr(0x8E53));                /* "none" */
        else ds_cat_num(0xAC60, SW(0xC104));
        ds_cat(0xAC60, dstr(0x8E58));                                /* ".\n\n" */
    }
    ds_cat(0xAC60, dstr(0x8E5C));                                    /* "You collected " */
    ds_cat_num(0xAC60, l32(0xC092));
    ds_cat(0xAC60, fix && l32(0xC092) == 1 ? " talent in tribute, " : dstr(0x8E6B));   /* e2bd */
    ds_cat_num(0xAC60, l32(0xC096));
    ds_cat(0xAC60, fix && l32(0xC096) == 1 ? " talent in bets,\nand " : dstr(0x8E81)); /* e2ea */
    ds_cat_num(0xAC60, l32(0xC09A));
    ds_cat(0xAC60, fix && l32(0xC09A) == 1 ? " talent in plunder." : dstr(0x8E98));    /* e317 */
    stats_box(lh);
    ds_cpy(0xAC60, dstr(W(0x8CCE)));                                 /* the epilogue DS:8BD8 */
    stats_box(lh);
}

void race_victory_df58(void) {
    if (W(0xC1DE) == 0) { memmove(&g_ds[0xC1DA], &g_ds[0x9AB6], 18); setW(0xC1DE, 0x2687); }
    nv_target_screen_680F();
    music_load_5D73(0);
    /* 1D19 cursor hide (not ported) */
    stats_DF74();
    gallery_BDA4();
    parade_BF6A(1);
    gallery_free_C2C2();
    credits_B9A1();                                                  /* df70: thunk B0CE = section 1 B9A1 (credits.c) */
}
