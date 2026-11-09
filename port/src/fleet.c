/* Fleet orders: selecting a Roman fleet on the map, its info box, moving it, the fights at sea it meets, and the two
 * helpers End Turn uses when an enemy army sails into a Roman fleet's sea. Claude, 2026-11-09. Literal translations of
 * EXE segment 0823 (disasm/EXE_RUNTIME.clean.asm), each with its address; docs/FLEET_NOTES.md sections 2-7 and 9.
 *
 *   fleet_select_2c6f   0x2C6F  (with the table 0x2DB2; fleets_in 0x2D60 and fleet_row 0x2E96 are province.c's)
 *   fleet_info_2f5a     0x2F5A
 *   fleet_move_2b40     0x2B40  (the map loop calls 2C6F, 3F23, 2F5A, 2B40 at 0x08C9..0x08EA, map.c)
 *   fleet_fight_armies_4ada  0x4ADA  every army at sea in the target fights the fleet (B12A via thunk B0C2, naval.c)
 *   fleet_army_to_record6_531f 0x531F  an army at sea becomes fleet record 6 for the naval battle
 *   fleet_retreat_5244  0x5244  End Turn: a Roman fleet that retreated from a naval battle (enemy.c, 0x51C5..0x523C)
 *
 * The naval battle (section 5 B0E0 / B12A) is in naval.c. Messages of the logic routines (4BAC, 5244's 59D4) go through
 * game_message (endturn.c: a box when a screen is open, else stderr), so End Turn runs headless as before. The mouse
 * cursor calls (1D55 / 1D8A / 1CCE) and the disk check 1E21 do nothing in the port (house style).
 *
 * 1990 bugs fixed by default, the original kept under --original (g_original_bugs), FLEET_NOTES section 9:
 *   9.1 5244 "nowhere to retreat and is destroyed" left the three ship counts (and +9) in the record, and the next Build
 *       Fleet (0x2458) reuses that record with its counts: the "destroyed" fleet came back. Fixed: counts and +9 cleared.
 *   9.2 531F divides by zero (0x53AF `div di`, a CPU divide error in DOS) when an army at sea is smaller than one ship of
 *       the rolled type (ships = 0). The port takes 1 ship in both modes (a crash cannot be reproduced) and says so.
 *   9.3 After a result 3 (both fleets gone; bit 1 is set, so the loops go on) 4ADA fights the next army in z, and then
 *       B0E0 any enemy fleet in z, with a Roman fleet of no ships. Fixed: 4ADA and B0E0 start no battle once the Roman
 *       record is gone (+8 = FFh); the return values stay the original's (4ADA FFFF, B0E0 1), so 2B40 still writes z
 *       into the dead fleet's +6 and into the zone byte (+0x0C, 1924) of the legion 4816 has just destroyed (FFh -> z).
 *       That write is a 1990 quirk kept in both modes: every reader tests the fleet's +8 / the legion's +0x0D first.
 * Not changed (reported to Erik): Esc at the flagship choice during End Turn (B12A result 0) dodges the attack (9.4).
 */
#include <stdio.h>
#include <string.h>
#include "screen.h"
#include "state.h"
#include "legions.h"
#include "fleet.h"
#include "naval.h"

void game_message(const char *s);   /* endturn.c */

static const char *dstr(uint16_t a) { return (const char *)&g_ds[a]; }

/* ---------------------------------------------------------------- drawing helpers (on the screen, as after 680F) */
static ui_box_t box_of(int x0, int y0, int x1, int y1) { ui_box_t r = { x0, y0, x1, y1 }; return r; }
static void save691b(int x0, int y0, int x1, int y1) { ui_copy_rect(g_wrk, g_scr, box_of(x0, y0, x1, y1)); }
static void text899c(int x, int y, const char *s) { ui_text_attr(g_scr, x, y, s, W(0x9B69)); }          /* 0x899C(y, x, s) */
static void text8be4(int cx, int y, const char *s) { ui_text_block_attr(g_scr, cx, y, s, W(0x9B69)); }  /* 0x8BE4(y, x, s) */
static void box3134(int x0, int y0, int x1, int y1, int fill) { ui_box(g_scr, x0, y0, x1, y1, fill); }
/* 0x5614 menu_draw(rec, mode) on the live record */
static void menu_draw5614(uint16_t rec, int mode) { menu_t m; menu_load_ds(&m, rec); ui_menu_draw_mode(g_scr, &m, mode, -1); }
/* 0x55EA menu_on(rec): [rec+0x10] = FFFFh, 56B6 */
static void menu_on55ea(uint16_t rec) { menu_t m; setW((uint16_t)(rec + 0x10), 0xFFFF); menu_load_ds(&m, rec); ui_menu_enable(g_scr, &m); }
/* 0x55FF menu_off(rec): [rec+0x10] = 0, 57F7 */
static void menu_off55ff(uint16_t rec) { menu_t m; setW((uint16_t)(rec + 0x10), 0); menu_load_ds(&m, rec); ui_menu_disable(g_scr, &m); }
/* 0x54C8 menu_run(rec, pre): the preselection is ignored with a mouse (MENU_NOTES 4); item, or FFh = Esc / end of input */
static uint16_t menu_run54c8(uint16_t rec, uint16_t pre) {
    (void)pre;
    menu_t m;
    menu_load_ds(&m, rec);
    int r = menu_choose(&m, -1, -1);
    return r < 0 ? 0xFF : (uint16_t)r;
}
static void msgbox59d4(uint16_t s) { game_message(dstr(s)); }

/* ---------------------------------------------------------------- 0x2DB2 fleet_table(title, z), ret 4 */
static void fleet_table_2db2(uint16_t title, uint8_t z) {
    /* 1D55(0) */
    uint8_t n = fleets_in2d60(z);                                             /* [bp-3] */
    if (n == 0) return;                                                       /* 2DCE -> 2E8B */
    if (W(0x1B90) == 0) {                                                     /* the box is not up yet */
        uint8_t lh = (uint8_t)(B(0x9B5C) + 2);                                /* [bp-2] */
        uint16_t si = 5;
        if (title != 0) si = (uint16_t)(lh + 5);
        if (n > 3) n = 3;
        B(0x1B8C) = n;                                                        /* menu record 1B7E: item count */
        uint16_t h = (uint16_t)((uint16_t)(n + 1) * lh + si + 7);             /* 2E06..2E12 */
        setW(0x1B84, h);                                                      /* record 1B7E: Y1 */
        setW(0x1B52, h);                                                      /* 3F23's restore rect */
        save691b(2, 0, 0x13D, h);
        menu_draw5614(0x1B7E, 2);
        setW(0x9B69, 0);
        text899c(0x50, si, dstr(0x1314));                                     /* "Triremes  Quinqueremes  Galleons   Legion   Status" */
        si = (uint16_t)(si + lh);
        for (uint8_t i = 0; i < n; i++) {                                     /* [bp-1] */
            province_fleet_row(g_scr, si, B(0xA69E + i));                     /* 2E96 */
            si = (uint16_t)(si + lh);
        }
    }
    setW(0x9B69, 0xF);
    text8be4(0xA0, 5, dstr(title));                                           /* title 0 = DS:0000 = "" */
    setW(0x1B90, 0xFFFF);
    /* 1D8A */
}

/* ---------------------------------------------------------------- 0x2C6F select_fleet(z), ret 2 */
uint8_t fleet_select_2c6f(uint8_t z) {
    uint8_t pick = 3;                                                         /* [bp-2]: nothing chosen yet */
    uint16_t si = 0;
    uint8_t n = fleets_in2d60(z);                                             /* [bp-1] */
    if (n == 0) return 0xFF;
    if (n > 3) n = 3;
    B(0x1B8C) = n;
    /* 1D55(0) */
    if (n > 1 || B(12 * B(0xA69E) + 0x1116) != 0)                             /* several, or the only one has acted */
        fleet_table_2db2(B(0xA69C) != 0 ? 0x1390 : 0, z);                     /* "Select Fleet" when one is Ready */
    if (n > 1 && B(0xA69C) != 0) {                                            /* 2CD0: a choice to make */
        menu_on55ea(0x1B7E);
        screen_present();
        do {
            si = menu_run54c8(0x1B7E, si);
            if (si == 0xFF) pick = 0xFF;                                      /* Esc */
            else if (B(12 * B(0xA69E + si) + 0x1116) == 0) pick = B(0xA69E + si);
            else msgbox59d4(0x1348);                                          /* "Fleet has already\nacted this turn" */
        } while (pick == 3);
        menu_off55ff(0x1B7E);
    } else {                                                                  /* 2D2A */
        pick = B(0xA69E);
        if (B(12 * pick + 0x1116) != 0) {
            screen_present();
            msgbox59d4(n > 1 ? 0x136A : 0x1348);                              /* "All fleets here have\nacted..." / "Fleet has..." */
            pick = 0xFF;
        }
    }
    /* 1D8A */
    screen_present();
    return pick;
}

/* ---------------------------------------------------------------- 0x2F5A fleet_info(f), ret 2 */
/* The same frame as legions_move_header 3EA3, but W(1B90) is not set: the box goes with fleet_move's closing map_build. */
void fleet_info_2f5a(uint8_t f) {
    setW(0x1B52, (uint16_t)((B(0x9B5C) + 6) * 2 + B(0x9B5D)));
    /* 1D55(0) */
    save691b(2, 0, 0x13D, W(0x1B52));
    box3134(2, 0, 0x13D, W(0x1B52), W(0x6346));
    setW(0x9B69, 0);
    text899c(0x50, 5, dstr(0x1314));
    province_fleet_row(g_scr, B(0x9B5C) + B(0x9B5D) + 5, f);                 /* 2E96 */
    /* 1D8A */
    screen_present();
}

/* ---------------------------------------------------------------- 0x2B40 fleet_move(f), ret 2 */
void fleet_move_2b40(uint8_t f) {
    uint8_t dest = 0xFF;                                                      /* [bp-4]: map_build's pointer warp */
    uint16_t done = 0;                                                        /* di */
    uint16_t rec = (uint16_t)(12 * f);                                        /* [bp-8] */
    uint8_t z0 = B(rec + 0x1114);                                             /* [bp-3] */
    /* 1D55(0) */
    for (int k = 0; k < 8; k++) {                                             /* markers on the neighbouring seas only */
        uint8_t a = B((uint16_t)(0x50EC + (z0 << 3) + k));                    /* [bp-1] */
        if (a < 0x1B || a == 0xFF) continue;
        map_sprite_8e62(g_scr, W(0xC0F2), SW(0x1AB8 + 4 * a), SW(0x1ABA + 4 * a));   /* 8E62(y, x, [C0F2]) */
    }
    screen_present();                                                         /* 1CCE */
    while (!done) {
        wait_release();                                                       /* 35DB */
        if (wait_input() == 0) {                                              /* 359B: a key */
            if (B(0xC07F) == 0x1B || screen_quit()) done = 0xFFFF;            /* Esc (the port: or the input ended) */
            continue;
        }
        uint8_t z = (uint8_t)zone_at(SW(0xB6), SW(0xB8));                     /* 6C67, either button */
        if (z == z0) done = 0xFFFF;                                           /* its own sea: cancel */
        if (z < 0x1B || z == 0xFF) continue;
        int k = 0;
        while (k < 8 && B((uint16_t)(0x50EC + (z0 << 3) + k)) != z) k++;
        if (k == 8) continue;                                                 /* not a neighbour */
        B(rec + 0x1116) = 1;                                                  /* "Moved", before any fight */
        /* 1E21 */
        if (fleet_fight_armies_4ada(f, z) != 0 && naval_enter_b0e0(f, z) == 1) {   /* 2C15; thunk B09E: `dec ax; jne` */
            B(rec + 0x1114) = z;                                              /* 2C30: the fleet moves */
            uint8_t L = B(rec + 0x1117);                                      /* [bp-2] */
            if (L != 0xFF) B((uint16_t)(20 * L + 0x1924)) = z;                /* and the legion aboard */
        }
        done = 0xFFFF;
        dest = z;                                                             /* the pointer goes to z even after a failed move */
    }
    map_build(dest);                                                          /* 494A */
    /* 1D8A */
}

/* ---------------------------------------------------------------- 0x4ADA fight_armies_at_sea(f, z), ret 4 */
uint16_t fleet_fight_armies_4ada(uint8_t f, uint8_t z) {
    uint16_t r = 0xFFFF;                                                      /* [bp-2] */
    uint16_t di = 0;
    for (uint16_t si = 0xC11A; si < 0xC1B4; si += 0x0E, di++) {               /* army records 0..10, si = A+8 */
        if (W(si) == 0 || B(si - 7) != z) continue;                           /* active, A+1 == z */
        if (!g_original_bugs && B(12 * f + 0x1116) == 0xFF) {                 /* 9.3: a result 3 before, the fleet is gone */
            fprintf(stderr, "fleet: fleet %u is gone; the original would fight army %u at sea with no ships\n", f, di);
            break;                                                            /* r stays FFFF, as the original returns */
        }
        army_meets_4bac(W(0x1100 + 2 * f), 0, z, di);                         /* 4B11: push di, z, 0, name */
        /* 1E21 */
        fleet_army_to_record6_531f(di);
        if (!(naval_battle_b12a(6, f) & 2)) { r = 0; break; }                 /* 4B22 thunk B0C2: push f, 6 */
    }
    return r;
}

/* ---------------------------------------------------------------- 0x531F army_to_record6(i), ret 2 */
void fleet_army_to_record6_531f(uint16_t i) {
    uint16_t t = rng_roll(3);                                                 /* [bp-4]: the ship type */
    uint16_t a = (uint16_t)(14 * i);
    uint16_t total = (uint16_t)((((uint16_t)B(a + 0xC11C) * 2 + W(a + 0xC116)) << 2) + W(a + 0xC118));   /* [bp-8], 16-bit */
    uint16_t q = (uint16_t)(total / B(0x8EDE + t));                           /* capacity 50 / 100 / 200 */
    uint16_t n = (uint16_t)(q + (q >> 3));                                    /* di: 9/8 of the ships that carry it */
    if (n == 0) {                                                             /* 9.2: 0x53AF divides by zero */
        fprintf(stderr, "fleet: army %u at sea (%u men) fills no %s; the original divides by zero here (crash), the port "
                        "takes 1 ship\n", i, total, dstr(W(0x9722 + 2 * t)));
        n = 1;
    }
    for (uint16_t bx = 0; bx < 3; bx++) setW((uint16_t)(0x1156 + 2 * bx), t == bx ? n : 0);
    B(0x115C) = B(a + 0xC113);                                                /* zone */
    B(0x115E) = 0;                                                            /* status */
    B(0x115F) = (uint8_t)i;                                                   /* the marines: army i */
    B(0x1160) = (uint8_t)(total / n);                                         /* men per ship (al) */
}

/* ---------------------------------------------------------------- 0x5244 fleet_retreat(k), ret 2 */
void fleet_retreat_5244(uint8_t k) {
    uint16_t ok = 0;                                                          /* [bp-0xC] */
    uint16_t rec = (uint16_t)(12 * k);                                        /* [bp-0xE] */
    uint8_t z = B(rec + 0x1114);                                              /* [bp-6] */
    uint8_t d = B(0x3D55 + z);                                                /* [bp-7] */
    for (int di = 0; di < 8; di++) {
        uint8_t a = B((uint16_t)(0x50EC + (z << 3) + di));                    /* [bp-1] */
        if (a == 0xFF || a < 0x1B) continue;                                  /* seas only */
        if (B(0x3D55 + a) > d) continue;                                      /* unsigned: an equal or weaker sea */
        uint8_t to = a;                                                       /* [bp-5] */
        for (uint16_t si = 0x1138; si < 0x115C; si += 0x0C)                   /* enemy fleets 3..5 */
            if (B(si) == a && B(si + 2) != 0xFF) to = 0xFF;
        if (to == 0xFF) continue;
        B(rec + 0x1114) = to;
        uint8_t L = B(rec + 0x1117);
        if (L != 0xFF) B((uint16_t)(20 * L + 0x1924)) = to;
        ok = 0xFFFF;
        break;
    }
    if (ok == 0) {
        msgbox59d4(0x3DCE);                                                   /* "Your fleet has\nnowhere to retreat\nand is destroyed!" */
        B(rec + 0x1116) = 0xFF;
        if (B(rec + 0x1117) != 0xFF) legion_lost(B(rec + 0x1117));            /* 4816 */
        if (!g_original_bugs) {                                               /* 9.1: the ships do not come back */
            setW(rec + 0x110E, 0); setW(rec + 0x1110, 0); setW(rec + 0x1112, 0);
            B(rec + 0x1117) = 0xFF;
        }
    }
}
