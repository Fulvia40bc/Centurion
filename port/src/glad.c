/* The gladiator show, OVL0 section 3 (file 0x3EF0-0x51A0, loaded at 0823:B0E0), Claude 2026-11-10.
 * Translated literally from disasm/OVL_GLAD.asm (addresses in the comments), after docs/GLAD_INDEX.md, GLAD_LOGIC_NOTES.md,
 * GLAD_SCREEN_NOTES.md and GLAD_DATA_NOTES.md. The drawing routines are in glad_gfx.c (on naval_gfx.c's EXE helpers).
 *
 *   B0E0  the show (entry, thunk B0B6 from Rome Games 0x34F5)     B38B  selection (B4DA pick + training, B4A3 release)
 *   B7FE  the fight: 4-tick frames, BBC8 exchange, BC4C AI, BB3B crowd   BDEA  outcome, walk-over, thumbs vote; C0F9 death
 *   C288  set-up of the two fighter blocks (P = DS:AA52 the player's man on the right, O = DS:AA6A the opponent)
 *   drawing (glad_gfx.c): B67C grid, B6EC card, C149 crowd bar, C1A4 fighters, C25F / C302 health bars, 3233 box
 *
 * TIME. The fight runs in frames of exactly 4 timer ticks (busy-wait after the drawing, B9E3..BAD9), the walk-over in
 * 4-tick steps, the title waits 432 ticks; the dice per frame do not depend on the CPU speed. The port keeps the
 * original's clock in the DS (DS:42/44 and the slots DS:46.., naval.c's nv_t_* = EXE 00D7/00F8/0183/01AE) and runs its
 * busy-waits with naval.c's nv_tick(): one tick passes per call, in the window at 72.8 Hz real time, headless (--script)
 * at once. Input in the fight: the held arrows DS:A2 (screen_held_dirs; script lines "hold KEY N" hold a key for N
 * ticks = N / 4 fight frames, "wait N" waits N ticks), read when the player's input sample fires; typed keys are read
 * and dropped every frame (b8b9), as in 1990. The vote reads the mouse (script "click X Y") and the Up / Down arrows;
 * one tick passes per pass of the vote and typed keys are read and dropped (script "wait N" / "key K" lines run there).
 *
 * 1990 BUGS fixed by default (--original keeps each, unless said otherwise):
 *   1. No refund when the opponent is cancelled (LOGIC 12.1): after the player paid his own man's training (b649), Esc in
 *      the opponent's training menu went b48b -> b45f past the refund b459: 10..40 talents lost, no show held, and as
 *      154C is not written the show could be started again (paying again). Fix: the refund (b459) on that path too.
 *   2. A picked gladiator leaked status 1 (LOGIC 12.2): b579 sets status 1 at the click, before the training menu, and the
 *      card's slot is overwritten by the beast, so Esc in that menu left the record at 1 for good (saved; never offered
 *      again; after 5 leaks the line-up's probe loop falls through, below). Fix: B4DA frees the record when it returns
 *      FF, and B38B first sets every record 0..12 at status 1 or 3 back to 0 (those exist only during a show; this
 *      repairs saves made by 1990 or under --original, no dice). Without that repair, 5 leaks left a slot whose probe
 *      loop found no free record (b3e4); it holds the record whose card is shown (si), which can be in the line-up
 *      already, so one man could fight himself (V by a headless run on a save with records 0..4 at status 1). In 1990
 *      that slot held an uninitialised stack byte (not reproducible: documented divergence, --original only).
 *   3. A click between the cards hung the game (LOGIC 12.3, SCREEN 6.2): `armed` ([bp-8]) is set by the first click and
 *      never cleared, so after a miss B4DA hit-tested the stale position forever (35DB polls only while a button is
 *      down). Fixed in BOTH modes (a hang has nothing to keep): a miss waits for the next click.
 *   4. Esc on the card grid did nothing (SCREEN 6.2): the Esc test b505 sits on the button branch of 359B, where [C07F]
 *      is always 0, and a key loops back to the wait (b501 je b514). Fix: Esc (a key) cancels the pick, as the code meant.
 *      (GLAD_LOGIC_NOTES 3 reads "any key acts as a click; Esc cancels before the first click"; the bytes say otherwise:
 *      keys do nothing in 1990.)
 *   5. The title ignored mouse clicks (SCREEN 6.1, LOGIC 12.7): b1bb tests [B2] without polling the mouse (0644), so only
 *      a key or the 5.9 s timeout ended it. Fix: 0644 is polled in the loop.
 *   6. The crowd meter stopped at x 220 (SCREEN 4.8; glad_gfx.c C149).
 *   7. The opponent man's high attack connected one animation step early (DATA 2.4 / 6.1, LOGIC 12.4; G): the move table
 *      O man (DS:7F50) has the hit flag 04 on state 10 (frame 3, the sword level at the shoulder), while the player man
 *      (7ED8) has it on state 11 (frame 4, the sword pointing up) and both wimp tables on their sword-up frame (3). The P
 *      and O man tables are otherwise exact mirrors and RMAN / LMAN mirrored sheets. Fix: O man state 10 flags 00, 11
 *      flags 04 (read through gl_flags; the DS bytes stay as they are). G: the two notes disagreed on which table is the
 *      odd one; the sheets (rendered) show the sword-up pose = frame 4 for the man and frame 3 for the wimp.
 *      ERIK'S CALL (open): GLAD_LOGIC_NOTES 12.4 proposed the OPPOSITE change (P man state 10 flags 04, state 11 flags
 *      00: the player's man faster, as 3 of the 4 tables hit on state 10); the port follows DATA 2.4 (the O man slower,
 *      the hit on the raised-sword pose as for the wimps, whose state 10 is their raised-sword frame). Both make the two
 *      men equal; this one makes the AI's high attack 6 frames slower, LOGIC's the player's 6 frames faster, 1990 has
 *      the AI faster by 6. The bytes support either reading; the default stays until Erik rules.
 * Kept (D, quirks): the AI pace DS:800D never reset (it carries over between shows); the beast records 10..12 not saved;
 * Spartacus' style 3 running style 2's code; the Up key changing only from stand; the contact coin every frame; attacks on
 * a falling man counting; an upset paying both ways (BDEA be8f); the last pass reading movetab[FF] (DS bytes past the
 * tables, read literally from the DS: no effect, LOGIC 12.5); the two empty "Click to continue" boxes (7BC6 / 7BC7).
 *
 * PORT ADDITIONS: the mouse cursor (1D55 / 1D8A / 1CCE / 1D19 / 1BE6) and the disk prompt are not drawn (the system
 * pointer); music is the DS part only (map.c / naval.c). When the input ends (window closed, script run out): in a card
 * pick or a training menu the selection is cancelled; in the fight the fight stops and the outcome (BDEA) is skipped
 * (154C is still written, as b313 comes before it); in the vote no vote is taken (no score, no kill). The original has no
 * way out of the fight or the vote.
 *
 * Easter eggs kept (all the data is the game's): Julius among Brutus, Cassius and Tillius (Caesar's assassins) and
 * Spartacus (style 3 = style 2); the bodies "man" and "wimp"; the training grades Trainee / Slave / Veteran / Master and,
 * for the big cats (Black Panther, Wild Lioness, Snow Leopard) that take the card you picked, Fat / Well-Fed / Hungry /
 * Ravenous; successors with numerals ("Flavius II"); the crowd that wants mercy after a good bout and blood after a dull
 * one; thumbs down on your own beaten man kills him; bouts lost, hits and blocks counted and never shown; "coloseum.cbm".
 *
 * NOT VERIFIED against DOSBox: no capture of section 3 exists; everything is D (decoded) and checked by headless runs.
 */
#include <stdio.h>
#include <string.h>
#include "state.h"
#include "screen.h"
#include "assets.h"
#include "naval_int.h"
#include "glad.h"

void music_load_5D73(uint8_t m);          /* map.c (the DS part of 5D73) */
void province_frame30e6(int x, int y);    /* province.c: 0x30E6 on the screen (the menu frame of 5614 mode 1) */

static int g_input_gone;                  /* port: the input ended in the fight or the vote */

static const char *dstr(uint16_t a) { return (const char *)&g_ds[a]; }
static void msg_59D4(uint16_t s) { message_box(dstr(s)); }

/* ---------------------------------------------------------------- music (the DS part, as naval.c) */
static void music_stop_5D20(void) { B(0x3EEE) = 0xFF; }
static void music_unload_5E1A(uint8_t n) {
    for (int i = 0; i < 4; i++) {
        if (B(0xA6B2 + 4 * i) == 0 || B(0xA6B3 + 4 * i) != n) continue;
        setW((uint16_t)(0xA6B0 + 4 * i), 0); B(0xA6B2 + 4 * i) = 0; B(0xA6B3 + 4 * i) = 0xFF; B(0x3EEE) = 0xFF;
        break;
    }
}

/* ---------------------------------------------------------------- move tables */
uint8_t gl_mt(uint16_t f, int k) {
    return B((uint16_t)(W(F_TAB(f)) + 4 * B(F_ST(f)) + k));          /* [movetab + 4 * (state & 0xFF) + k] */
}
static int8_t mt_dx(uint16_t f) { return (int8_t)gl_mt(f, 0); }
/* the flags byte; bug 7: the O man table (7F50) has its high-attack hit flag on state 10 instead of 11 */
uint8_t gl_flags(uint16_t f) {
    uint8_t v = gl_mt(f, 3);
    if (!g_original_bugs && W(F_TAB(f)) == 0x7F50) {
        if (B(F_ST(f)) == 10) v = 0x00;
        else if (B(F_ST(f)) == 11) v = 0x04;
    }
    return v;
}

/* ---------------------------------------------------------------- selection */
/* B4A3(lineup): status 3 -> 0 for the six line-up records */
static void release_B4A3(const uint8_t *lineup) {
    for (int si = 0; si < 6; si++)
        if (B(REC(lineup[si], R_STAT)) == 3) B(REC(lineup[si], R_STAT)) = 0;
}

/* B4DA(lineup): a click on a card (record status 1, the card becomes beast 10 + rng(3)), then "How Much Training?";
   the record, or FF (cancelled) */
static uint8_t pick_B4DA(uint8_t *lineup) {
    int done = 0, armed = 0;                                         /* [bp-2], [bp-8] */
    uint8_t rec = 0;                                                 /* [bp-5] */
    /* 1CCE cursor_show (not ported) */
    while (!done) {                                                  /* b4f3 */
        while (!armed) {                                             /* b514 */
            int b = wait_input();                                    /* b4fe: 359B */
            if (screen_quit()) return 0xFF;                          /* port: the input ended */
            if (b == 0) {                                            /* b501: a key (in [C07F]) */
                if (!g_original_bugs && B(0xC07F) == 0x1B) return 0xFF;   /* bug 4: Esc cancels (1990: keys do nothing) */
                continue;
            }
            if (B(0xC07F) == 0x1B) return 0xFF;                      /* b505 (never: a button leaves [C07F] = 0); 1D19 */
            armed = -1;                                              /* b50f */
        }
        for (int si = 0; si < 6; si++) {                             /* b51a */
            int16_t r[4];                                            /* [bp-14h] x0, y0, x1, y1 */
            r[1] = (int16_t)(0x64 * (si / 3) + 2);
            r[0] = (int16_t)(0x6A * (si % 3) + 7);
            r[2] = (int16_t)(r[0] + 0x5B);
            r[3] = (int16_t)(r[1] + 0x5B);
            if (!nv_inrect_8238(SW(0xB6), SW(0xB8), r)) continue;
            rec = lineup[si];                                        /* b56b */
            B(REC(rec, R_STAT)) = 1;                                 /* b579 */
            lineup[si] = (uint8_t)(rng_roll(3) + 10);                /* b582: a big cat takes the card's place */
            done = -1;
            break;
        }
        wait_release();                                              /* b591: 35DB */
        if (!done) armed = 0;                                        /* bug 3 (both modes): a miss waits for a new click */
    }
    /* b59f: 1D19; the four item texts ([bp-78h] + 25k), " (n0 Talents)" with the digit patched (DS:7B6A) */
    char item[4][0x19];
    for (int k = 0; k < 4; k++) {
        uint16_t lab = B(REC(rec, R_STY)) == 4 ? W(0x7C76 + 2 * k) : W(0x7C6E + 2 * k);
        snprintf(item[k], sizeof(item[k]), "%s", dstr(lab));        /* 9E32 */
        B((uint16_t)(W(0x7C7E) + 2)) = (uint8_t)('1' + k);           /* b5dc */
        size_t l = strlen(item[k]);
        snprintf(item[k] + l, sizeof(item[k]) - l, "%s", dstr(W(0x7C7E)));   /* 9DF2 */
        /* b5f0: [7C80 + 12k] = the stack buffer's address: not reproduced (the menu gets the texts below) */
    }
    menu_t m;
    menu_load_ds(&m, 0x7CB0);
    for (int k = 0; k < 4 && k < m.count; k++) snprintf(m.items[k].text, sizeof(m.items[k].text), "%s", item[k]);
    province_frame30e6(m.x, m.y);                                    /* 5614(7CB0, 1): the frame, then the rest */
    ui_menu_draw_mode(g_scr, &m, 0, -1);
    /* 1CCE */
    int got = 0;                                                     /* si = [bp-0Ah] */
    while (!got) {                                                   /* b66d */
        int sel = menu_choose(&m, -1, -1);                           /* b617: 54C8(7CB0, di) */
        if (sel < 0) {                                               /* b621: FF */
            if (!g_original_bugs) B(REC(rec, R_STAT)) = 0;           /* bug 2: the picked record is free again */
            got = -1; rec = 0xFF;                                    /* b666 */
            break;
        }
        if ((uint16_t)(10 * (sel + 1)) > W(0x634A)) {                /* b62e: ja */
            ui_menu_highlight(g_scr, &m, sel, 0);                    /* 585B(7CB0, di, 0) */
            msg_59D4(0x7D7D);                                        /* "You Cannot Afford Such\na Well-Trained Gladiator" */
            continue;
        }
        B(REC(rec, R_TRN)) = (uint8_t)sel;                           /* b63d */
        setW(0x634A, (uint16_t)(W(0x634A) - 10 * (sel + 1)));        /* b649 */
        got = -1;
    }
    return rec;
}

/* B38B: -1 = a show, 0 = none (cannot afford, or cancelled) */
static int select_B38B(void) {
    uint8_t lineup[6];                                               /* [bp-0Eh] */
    if (!g_original_bugs)                                            /* bug 2: statuses 1 / 3 exist only during a show; */
        for (int r = 0; r < 13; r++)                                 /* a save from 1990 or --original may carry leaks */
            if (B(REC(r, R_STAT)) == 1 || B(REC(r, R_STAT)) == 3) B(REC(r, R_STAT)) = 0;
    if (W(0x634A) < 0x14) { msg_59D4(0x7CC2); return 0; }            /* "You Cannot Afford\nTwo Gladiators." */
    uint16_t saved = W(0x634A);                                      /* [bp-8] */
    gl_grid_B67C();
    for (unsigned slot = 0; slot < 6; slot++) {                      /* [bp-6] */
        uint16_t r = rng_roll(10);                                   /* [bp-2] = [bp-4] */
        uint16_t si = r;
        int found = 0;
        do {                                                         /* b3c3 */
            if (B(REC(si, R_STAT)) == 0) { found = 1; break; }
            si = (uint16_t)((si + 1) % 10);
        } while (si != r);
        if (found) { lineup[slot] = (uint8_t)si; B(REC(si, R_STAT)) = 3; }   /* b3e6 */
        else lineup[slot] = (uint8_t)si;                             /* b3e4: 1990 leaves the stack byte (bug 2) */
        gl_card_B6EC((int)slot, (uint8_t)si, -1);                    /* b3f7 */
    }
    msg_59D4(0x7CE4);                                                /* "Select a Gladiator\nfor You to Control" */
    B(0xAA51) = pick_B4DA(lineup);
    if (B(0xAA51) == 0xFF) { release_B4A3(lineup); /* 1D19 */ return 0; }
    /* 1D19 */
    gl_grid_B67C();
    for (int si = 0; si < 6; si++) gl_card_B6EC(si, lineup[si], 0);  /* facing left: your card now shows a beast */
    /* 1CCE */
    if (W(0x634A) < 0x0A) {
        msg_59D4(0x7D0A);                                            /* "You Cannot Afford a Second Gladiator.\n..." */
        setW(0x634A, saved);                                         /* b459 */
        goto back_out;
    }
    msg_59D4(0x7D59);                                                /* "Select a Gladiator\nTo Fight Against" */
    B(0xAA50) = pick_B4DA(lineup);
    if (B(0xAA50) == 0xFF) {                                         /* b48b: 1D19, jmp b45f */
        if (!g_original_bugs) setW(0x634A, saved);                   /* bug 1: the refund of b459 */
        goto back_out;
    }
    release_B4A3(lineup);
    /* 1D19 */
    return -1;
back_out:                                                            /* b45f */
    B(REC(B(0xAA51), R_STAT)) = 0;
    release_B4A3(lineup);
    return 0;
}

/* ---------------------------------------------------------------- the fight */
/* C288(player, opp): training, states 0, move tables and dying states by body, P (192, 175), O (127, 175) */
static void setup_C288(uint8_t p, uint8_t o) {
    B(F_TRN(GL_P)) = B(REC(p, R_TRN));
    B(F_TRN(GL_O)) = B(REC(o, R_TRN));
    B(F_ST(GL_O)) = 0; B(F_ST(GL_P)) = 0;
    uint8_t bp_ = B(REC(p, R_BODY)), bo = B(REC(o, R_BODY));
    setW(F_TAB(GL_P), W(0x7FE8 + 2 * bp_));
    setW(F_TAB(GL_O), W(0x7FEC + 2 * bo));
    B(F_DIE(GL_P)) = B(0x7ED0 + bp_);
    B(F_DIE(GL_O)) = B(0x7ED2 + bo);
    setW(F_X(GL_P), 0xC0); setW(F_X(GL_O), 0x7F);
    setW(F_Y(GL_O), 0xAF); setW(F_Y(GL_P), 0xAF);
}

/* BB3B(delta): the crowd meter 0..30 and its bar; 4 exchanges in a row -> mood up, 4 dull spells -> mood down (music) */
static void crowd_BB3B(int16_t d) {
    setW(0xAA62, (uint16_t)(W(0xAA62) + d));
    if (SW(0xAA62) < 0) setW(0xAA62, 0);                             /* jns */
    else if (SW(0xAA62) > 0x1E) setW(0xAA62, 0x1E);
    gl_crowd_C149(SW(0xAA62));
    if (d > 0) {
        setW(0xAA68, 0);
        setW(0xAA60, (uint16_t)(W(0xAA60) + 1));
        if (SW(0xAA60) <= 3) return;
        setW(0xAA60, 0);
        setW(0xAA66, (uint16_t)(W(0xAA66) + 1));
        if (SW(0xAA66) > 2) setW(0xAA66, 2);
    } else {
        setW(0xAA60, 0);
        setW(0xAA68, (uint16_t)(W(0xAA68) + 1));
        if (SW(0xAA68) <= 3) return;
        setW(0xAA68, 0);
        setW(0xAA66, (uint16_t)(W(0xAA66) - 1));
        if (SW(0xAA66) < 0) setW(0xAA66, 0);                         /* jns */
    }
    music_play_5D29(B((uint16_t)(0x800A + W(0xAA66))));             /* bbb6 */
}

/* BBC8(attacker, defender): an attack zone against the defender's guard within 20 px: a hit (health -= training + 1) or
   a block; either way crowd +1 and the boredom timer (slot 1) restarted */
static void exchange_BBC8(uint16_t a, uint16_t d) {
    uint8_t z = (uint8_t)(gl_flags(a) & 6);                          /* [bp-1] */
    if (!z) return;
    int16_t dist = (int16_t)(SW(F_X(d)) - SW(F_X(a)));
    if (dist < 0) dist = (int16_t)-dist;
    if (dist >= 0x14) return;
    uint8_t g = (uint8_t)((int8_t)(gl_flags(d) & 0x18) >> 2);        /* sar al, 2 */
    if (g != z) {
        setW(0xAA5E, (uint16_t)(W(0xAA5E) + 1));                     /* hits (never read) */
        B(F_HP(d)) = (uint8_t)(B(F_HP(d)) - (uint8_t)(B(F_TRN(a)) + 1));
    } else setW(0xAA64, (uint16_t)(W(0xAA64) + 1));                  /* blocks (never read) */
    crowd_BB3B(1);
    nv_t_start_00D7(1);
}

/* BC4C(opp): the opponent's next state by style, paced by DS:800D += training + difficulty DS:0490 + 5 (acts at 30) */
static void ai_BC4C(uint8_t opp) {
    int di = 0, si = 0;                                              /* medium, close */
    if (SB(F_HP(GL_O)) <= 0) { B(F_ST(GL_O)) = B(REC(opp, R_STY)) == 4 ? 4 : 9; return; }
    B(0x800D) = (uint8_t)(B(0x800D) + (uint8_t)(B(F_TRN(GL_O)) + B(0x0490) + 5));
    if (B(0x800D) < 0x1E) return;
    B(0x800D) = 0;
    if ((int16_t)(SW(F_X(GL_O)) + 0x14) >= SW(F_X(GL_P))) si = -1;   /* bc95 */
    if ((int16_t)(SW(F_X(GL_O)) + 0x24) >= SW(F_X(GL_P)) && !si) di = -1;
    uint16_t sty = REC(opp, R_STY);                                  /* [bp-2] */
    uint8_t st = B(F_ST(GL_O));
    switch (B(sty)) {
    case 0:                                                          /* bcdf */
        if (si) st = rng_roll(2) ? 3 : 4;
        else st = 1;
        break;
    case 1:                                                          /* bcf4 */
        if (di) st = 1;
        else if (si) st = B((uint16_t)(0x7FF6 + rng_roll(10)));
        else st = 2;
        break;
    case 2: case 3:                                                  /* bd14 */
        if (si) st = B((uint16_t)(0x8000 + rng_roll(10)));
        else if (di) st = 1;
        else { uint16_t r = rng_roll(4); st = r < 2 ? 5 : r == 3 ? 7 : 6; }
        break;
    case 4:                                                          /* bd58 */
        if (SB(F_HP(GL_O)) > 5) st = di ? 1 : si ? 3 : 1;
        else if (SW(F_X(GL_O)) < 0x1E) st = 3;
        else st = si ? 3 : 2;
        break;
    default: break;                                                  /* bd83 */
    }
    B(F_ST(GL_O)) = st;
    if (B(sty) != 4) {                                               /* bd83 */
        if (B(F_ST(GL_O)) == 1 && SW(F_X(GL_O)) > 0x122) B(F_ST(GL_O)) = 4;
        if (B(F_ST(GL_O)) == 2 && SW(F_X(GL_O)) < 0x1E) B(F_ST(GL_O)) = rng_roll(2) ? 6 : 5;
    } else if ((B(F_ST(GL_O)) == 1 && SW(F_X(GL_O)) > 0x122) || (B(F_ST(GL_O)) == 2 && SW(F_X(GL_O)) < 0x1E))
        B(F_ST(GL_O)) = 3;
}

/* B7FE(opp): the fight, until a fighter's fall reaches FF */
static void fight_B7FE(uint8_t opp) {
    int16_t Ptick = 6, Otick = 6;                                    /* [bp-0Ah], [bp-0Ch] */
    int end = 0, Pmoved = 0, Omoved = 0;                             /* [bp-4], [bp-10h], [bp-0Eh] */
    int16_t Pcmd = 0, Ocmd = 0;                                      /* [bp-8], [bp-6] */
    setW(0xAA68, 0); setW(0xAA60, 0); setW(0xAA5E, 0); setW(0xAA64, 0);
    setW(0xAA66, 1);                                                 /* mood 1 */
    B(F_HP(GL_O)) = 10; B(F_HP(GL_P)) = 10;
    for (uint16_t i = 0; B(F_TRN(GL_P)) > i; i++) B(F_HP(GL_P)) = (uint8_t)(B(F_HP(GL_P)) + rng_roll(10));
    for (uint16_t i = 0; B(F_TRN(GL_O)) > i; i++) B(F_HP(GL_O)) = (uint8_t)(B(F_HP(GL_O)) + rng_roll(10));
    gl_health_C25F(1);
    gl_health_C25F(0);
    nv_t_start_00D7(1);
    setW(0xAA62, (uint16_t)(B(F_TRN(GL_O)) + B(F_TRN(GL_P)) + 2));   /* the crowd meter */
    gl_box_3233(0x4F, 0x0A, 0xEA, 0x14, SW(0x6346));
    gl_crowd_C149(SW(0xAA62));
    while (!end) {                                                   /* bb24 */
        key_read();                                                  /* b8b9: 03AA, the key dropped */
        key_flush();                                                 /* 04ED */
        if (screen_quit()) { g_input_gone = 1; break; }              /* port: the input ended */
        if (B(F_ST(GL_P)) == 0xFF || B(F_ST(GL_O)) == 0xFF) end = -1;   /* b8bf */
        if (Pcmd <= 0) {                                             /* b8d2: the player's sample */
            Pcmd = (int16_t)((4 - B(F_TRN(GL_P))) << 2);
            if (gl_flags(GL_P) & 1) {
                if (SB(F_HP(GL_P)) <= 0) B(F_ST(GL_P)) = 9;          /* b987: fall */
                else {
                    B(0xA2) = (uint8_t)screen_held_dirs();           /* the INT 9 hook's live arrow bits */
                    switch (B(0xA2)) {                               /* b909: jump table B91C */
                    case 1:  if (SW(F_X(GL_P)) < 0x122) B(F_ST(GL_P)) = 2; break;   /* Right: step back */
                    case 2:  if (SW(F_X(GL_P)) > 0x1E) B(F_ST(GL_P)) = 1; break;    /* Left: step in */
                    case 4:  B(F_ST(GL_P)) = 5; break;               /* Down: low guard */
                    case 6:  B(F_ST(GL_P)) = 4; break;               /* Down+Left: low attack */
                    case 8:                                          /* Up: high guard, from stand only */
                        if (B(F_ST(GL_P)) == 0) { B(F_ST(GL_P)) = 7; if (rng_roll(2)) B(F_ST(GL_P)) = 6; }
                        break;
                    case 10: B(F_ST(GL_P)) = 3; break;               /* Up+Left: high attack */
                    default: B(F_ST(GL_P)) = 0; break;               /* b930 */
                    }
                }
                B(F_ST2(GL_P)) = B(F_ST(GL_P));                      /* b98c */
                Pmoved = -1;
            }
        }
        Pcmd--; Ptick--;                                             /* b997 */
        if (Ocmd <= 0) {                                             /* b99d: the AI's sample */
            Ocmd = (int16_t)((4 - B(F_TRN(GL_O))) << 1);
            if (gl_flags(GL_O) & 1) { ai_BC4C(opp); Omoved = -1; }
        }
        Ocmd--; Otick--;                                             /* b9d4 */
        if (end) break;                                              /* b9da: no frame on the last pass */
        nv_t_reset_00F8();                                           /* b9e3 */
        int16_t si, di;
        if (Pmoved) { si = (int16_t)(mt_dx(GL_P) + SW(F_X(GL_P))); Pmoved = 0; }   /* a state's dx once, on entry */
        else si = SW(F_X(GL_P));
        if (Omoved) { di = (int16_t)(mt_dx(GL_O) + SW(F_X(GL_O))); Omoved = 0; }
        else di = SW(F_X(GL_O));
        if (di < si) { setW(F_X(GL_P), (uint16_t)si); setW(F_X(GL_O), (uint16_t)di); }   /* ba36 */
        else if (rng_roll(2) == 0) { if (SW(F_X(GL_O)) < si) setW(F_X(GL_P), (uint16_t)si); }   /* contact: a coin */
        else if (SW(F_X(GL_P)) > di) setW(F_X(GL_O), (uint16_t)di);
        gl_fighters_C1A4();                                          /* ba61 */
        if (Ptick == 0) {                                            /* ba64 */
            exchange_BBC8(GL_P, GL_O);
            gl_health_C25F(0);
            B(F_ST(GL_P)) = gl_mt(GL_P, 1);
            Pmoved = -1; Ptick = 6;
        }
        if (Otick == 0) {                                            /* ba9b */
            exchange_BBC8(GL_O, GL_P);
            gl_health_C25F(1);
            B(F_ST(GL_O)) = gl_mt(GL_O, 1);
            Omoved = -1; Otick = 6;
        }
        while (nv_t_since0_01AE() < 4) nv_tick();                    /* bad3: the 4-tick frame */
        if (nv_t_since_0183(1) > 0xD8) { crowd_BB3B(-1); nv_t_start_00D7(1); }   /* badb: 3 s without an exchange */
        if ((B(F_ST(GL_P)) == 0xFF || B(F_ST(GL_O)) == 0xFF) &&      /* baf5 */
            (B(F_ST(GL_P)) == 0 || B(F_ST(GL_P)) == 0xFF) && (B(F_ST(GL_O)) == 0 || B(F_ST(GL_O)) == 0xFF))
            end = -1;
    }
    key_flush();                                                     /* bb2d */
    music_stop_5D20();
}

/* ---------------------------------------------------------------- the outcome */
/* C0F9(r): the successor: wins = losses = 0, generation + 1; a man gets a new body rng(2) and style rng(4); status 0 */
static void death_C0F9(uint8_t r) {
    B(REC(r, R_LOST)) = 0; B(REC(r, R_WON)) = 0;
    B(REC(r, R_GEN))++;
    if (B(REC(r, R_STY)) != 4) {
        B(REC(r, R_BODY)) = (uint8_t)rng_roll(2);
        B(REC(r, R_STY)) = (uint8_t)rng_roll(4);
    }
    B(REC(r, R_STAT)) = 0;
}

/* BDEA(player p [bp+6], opp o [bp+4]) */
static void outcome_BDEA(uint8_t p, uint8_t o) {
    int Pwon = 0, Owon = 0, killP = 0, killO = 0, up = 0, down = 0, upset = 0, pleased = 0;
    /* [bp-8], [bp-12h], [bp-0Ah], [bp-0Ch], [bp-0Eh], [bp-6], [bp-2], [bp-10h] */
    if ((int16_t)((B(F_TRN(GL_O)) + B(F_TRN(GL_P)) + 2) << 1) < SW(0xAA62)) pleased = -1;   /* be20 */
    if (SB(F_HP(GL_P)) > 0) {
        B(REC(p, R_WON))++;
        Pwon = -1;
        if (B(F_TRN(GL_O)) > B(F_TRN(GL_P))) { score_add(B(F_TRN(GL_O)) - B(F_TRN(GL_P))); upset = -1; }
        score_add(3);
    } else B(REC(p, R_LOST))++;
    if (SB(F_HP(GL_O)) > 0) {
        B(REC(o, R_WON))++;
        Owon = -1;
        if (B(F_TRN(GL_O)) < B(F_TRN(GL_P))) { score_add(B(F_TRN(GL_P)) - B(F_TRN(GL_O))); upset = -1; }   /* be8f */
        if (B(REC(o, R_STY)) == 4) killP = -1;                       /* the beast won: your man dies, no vote */
    } else {
        B(REC(o, R_LOST))++;
        if (B(REC(o, R_STY)) == 4) killO = -1;                       /* the beast lost: it dies, no vote */
    }
    if (upset) score_add(3);                                         /* bed3 */
    if (!Pwon && !Owon) { score_add(3); killO = killP = -1; goto deaths; }   /* both dead */
    if (killO || killP) goto deaths;
    int passed = 0;                                                  /* di = [bp-4] */
    do {                                                             /* bf17: the winner walks over to the loser */
        nv_t_reset_00F8();
        if (Pwon) {
            B(F_ST(GL_O)) = B(F_DIE(GL_O));
            if (gl_flags(GL_P) & 1) B(F_ST(GL_P)) = 1;
            setW(F_X(GL_P), (uint16_t)(W(F_X(GL_P)) + mt_dx(GL_P)));
        } else {
            B(F_ST(GL_P)) = B(F_DIE(GL_P));
            if (gl_flags(GL_O) & 1) B(F_ST(GL_O)) = 1;
            setW(F_X(GL_O), (uint16_t)(W(F_X(GL_O)) + mt_dx(GL_O)));
        }
        gl_fighters_C1A4();
        while (nv_t_since0_01AE() < 4) nv_tick();                    /* bf91 */
        if (SW(F_X(GL_O)) > SW(F_X(GL_P))) passed = -1;              /* bf99 */
        B(F_ST(GL_P)) = gl_mt(GL_P, 1);
        B(F_ST(GL_O)) = gl_mt(GL_O, 1);
    } while (!passed);
    gl_thumbs_box();                                                 /* bfd8 */
    gl_crowd_C149(SW(0xAA62));
    /* W(BA): 1CCE cursor_show (not ported) */
    msg_59D4(0x800E);                                                /* "Thumbs Up or Down?" */
    while (!up && !down) {                                           /* c004..c071 */
        if (W(0xBA)) {
            mouse_poll();                                            /* 0644 (1BE6, the cursor: not ported) */
            if (W(0xB2)) {
                int16_t ru[4], rd[4];
                for (int i = 0; i < 4; i++) { ru[i] = SW(0x7EC0 + 2 * i); rd[i] = SW(0x7EC8 + 2 * i); }
                if (nv_inrect_8238(SW(0xB6), SW(0xB8), ru)) up = -1;
                if (nv_inrect_8238(SW(0xB6), SW(0xB8), rd)) down = -1;
            }
        }
        B(0xA2) = (uint8_t)screen_held_dirs();
        if (B(0xA2) == 8) up = -1;                                   /* c050: Up alone */
        else if (B(0xA2) == 4) down = -1;                            /* Down alone */
        if (!up && !down && screen_quit()) { g_input_gone = 1; goto deaths; }   /* port: no vote */
        if (!up && !down) {               /* port: in 1990 the timer runs on and typed keys go into the BIOS buffer */
            key_read();                   /* (dropped by c073 anyway): a script's "key" line is taken here, and one */
            nv_tick();                    /* tick passes per pass (DS:42, the script's "wait" / "hold" lines run down) */
        }
    }
    key_flush();                                                     /* c073 */
    int v;
    if (down) {                                                      /* c076: Down wins over Up */
        if (Pwon) { killO = -1; B(F_ST(GL_P)) = 8; B(F_ST(GL_O)) = B(F_DIE(GL_O)); }   /* your man kills the loser */
        else if (Owon) { killP = -1; B(F_ST(GL_O)) = 8; B(F_ST(GL_P)) = B(F_DIE(GL_P)); }   /* your own man is killed */
        gl_fighters_C1A4();
        v = pleased ? -5 : 5;                                        /* c0ab */
    } else v = pleased ? 5 : -5;                                     /* c0b3 */
    crowd_BB3B((int16_t)v);
    score_add(v);
deaths:                                                              /* c0d3 */
    if (killO) death_C0F9(o);
    if (killP) death_C0F9(p);
}

/* ---------------------------------------------------------------- B0E0, the show */
void glad_show_b0e0(void) {
    /* the port's start-up state, as naval.c: the work buffer's descriptor and the screen as the target */
    if (W(0xC1DE) == 0) { memmove(&g_ds[0xC1DA], &g_ds[0x9AB6], 18); setW(0xC1DE, 0x2687); }
    nv_target_screen_680F();
    g_input_gone = 0;
    int si = assets_load("MGLAD.DIR", "MGLAD.DAT") == 0;             /* b0ef: 0F7B(6, &arch) = MGLAD */
    if (!si) { msg_59D4(0x7BA1); return; }                           /* b37f: "Can't Load Gladiator Graphics" */
    si &= gl_picture_13A8();                                         /* b103: arenabtm.cbm, picture slot 4 */
    if (si & gl_title_1273()) gl_title_draw();                       /* b114: coloseum.cbm */
    setW(SH_SHOW, nv_sheet_163B(dstr(0x7B96)));                      /* "show" */
    setW(SH_ARENA, nv_sheet_163B(dstr(0x7B9B)));                     /* "arena" */
    /* 1D55(0) cursor push (not ported) */
    setW(0x9B69, 0x0F);
    music_unload_5E1A(1);
    music_load_5D73(4); music_load_5D73(5); music_load_5D73(6); music_load_5D73(7);
    music_play_5D29(4);
    nv_t_reset_00F8();                                               /* b1a9 */
    for (;;) {                                                       /* b1ac: 432 ticks, a key or a button */
        if (nv_t_since0_01AE() >= 0x1B0) break;
        if (key_read() & 0xFF) break;                                /* 03AA */
        if (!g_original_bugs) mouse_poll_input();                    /* bug 5: 0644 polled (1990: [B2] is stale) */
        else if (!screen_scripted()) {                               /* --original: the window's events only */
            uint16_t b = W(0xB2), x = W(0xB6), y = W(0xB8);
            mouse_poll_input();
            setW(0xB2, b); setW(0xB6, x); setW(0xB8, y);
        }
        if (W(0xB2)) break;                                          /* b1bb */
        nv_tick();
    }
    music_play_5D29(7);
    if (select_B38B() == 0) {                                        /* b1c8 */
        music_unload_5E1A(4); music_unload_5E1A(5); music_unload_5E1A(6); music_unload_5E1A(7);
        nv_sheet_19A0(W(SH_SHOW));
        nv_sheet_19A0(W(SH_ARENA));
        /* 11A9 archive close, 1D8A cursor pop */
        music_stop_5D20();
        gl_pictures_free();
        return;
    }
    music_play_5D29(5);                                              /* b205 */
    gl_grid_B67C();
    nv_text_8BE4(dstr(0x7BBF), 0xA0, 0x7D);                          /* "VERSUS" */
    gl_card_B6EC(5, B(0xAA51), -1);
    gl_card_B6EC(3, B(0xAA50), 0);
    nv_sheet_19A0(W(SH_SHOW));
    setW(SH_PLAYER, nv_sheet_163B(dstr(W(0x7BCC + 2 * B(REC(B(0xAA51), R_BODY))))));   /* rman / rwimp */
    setW(SH_OPP, nv_sheet_163B(dstr(W(0x7BD0 + 2 * B(REC(B(0xAA50), R_BODY))))));      /* lman / lwimp / lcat0..2 */
    /* 11A9 archive close */
    music_play_5D29(4);
    msg_59D4(0x7BC6);                                                /* "": a bare "Click to continue" over VERSUS */
    gl_arena_draw();                                                 /* b29b */
    setup_C288(B(0xAA51), B(0xAA50));
    gl_fighters_C1A4();
    music_play_5D29(5);
    setW(0xC100, (uint16_t)(W(0xC100) + 1));                         /* b2f3: games sponsored (victory statistics) */
    if (SB(0x0ED4) > 0x0A) B(0x0ED4) = 0x0A;                         /* "Rome is bored": chance capped at 10 */
    setW(0x0ED8, 0);                                                 /* ... and the crisis ends */
    fight_B7FE(B(0xAA50));
    nv_target_screen_680F();
    setW(0x154C, W(0x634C));                                         /* b313: one show per turn */
    if (!g_input_gone) outcome_BDEA(B(0xAA51), B(0xAA50));           /* b321 */
    nv_target_screen_680F();
    music_stop_5D20();
    music_unload_5E1A(4); music_unload_5E1A(5); music_unload_5E1A(6); music_unload_5E1A(7);
    msg_59D4(0x7BC7);                                                /* "": the pause over the last scene */
    B(REC(B(0xAA51), R_STAT)) = 0;
    B(REC(B(0xAA50), R_STAT)) = 0;
    nv_sheet_19A0(W(SH_ARENA));
    nv_sheet_19A0(W(SH_PLAYER));
    nv_sheet_19A0(W(SH_OPP));
    /* 1D8A cursor pop */
    gl_pictures_free();
}
