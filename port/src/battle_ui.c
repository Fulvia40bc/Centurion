/* The land battle on screen: menus, the running battle, the orders mode, messages and the report.
 * Claude, 2026-11-09. The simulation (battle.c) and the drawing of the field and units (battle_gfx.c) are the
 * original's, verified; this file is the screen and input around them (D0F2/D198/D33A/D978, 54C8 menus, 59D4
 * messages, E21A report). Approximate where noted, to be checked against DOSBox captures.
 *
 * Timing as the original: one tick per 12 timer ticks of 72.8 Hz = 165 ms. During a tick, Space, Enter, '+', Insert or
 * a mouse click open the orders mode (the battle is frozen): buttons Select Roman Unit / Melee / Retreat / Done
 * (DS:748E rects, hotkeys S M R D, Esc = Done). Click a Roman cohort and drag to give it a destination; click it and
 * press Space (hold), M (melee) or R (retreat), or use the Melee / Retreat buttons for it. Without a selected cohort,
 * Melee = "Release The Legion!" and Retreat = "Run Away! Run Away!", both of which also end the orders mode.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "state.h"
#include "assets.h"
#include "platform.h"
#include "ui.h"

static uint8_t g_fb[SCREEN_W * SCREEN_H];
static rgb_t g_pal[256];
static uint8_t *g_ovl0;
static input_t g_in;
static int g_prev_buttons, g_quit;

static const char *dstr(uint16_t a) { return (const char *)&g_ds[a]; }

/* headless (battle_ui_shots): every presented frame can be saved instead of shown */
static const char *g_shot_prefix;
static int g_shot_n, g_headless;
static void shot(const char *tag) {
    if (!g_shot_prefix) return;
    char path[512];
    snprintf(path, sizeof(path), "%s_%02d_%s.bin", g_shot_prefix, g_shot_n++, tag);
    FILE *f = fopen(path, "wb");
    if (f) { fwrite(g_fb, 1, sizeof(g_fb), f); fclose(f); }
}
static void present(void) {
    if (g_headless) { g_prev_buttons = g_in.mouse_buttons; memset(&g_in, 0, sizeof(g_in)); g_in.mouse_buttons = 0; return; }
    platform_present(g_fb, g_pal);
    g_prev_buttons = g_in.mouse_buttons;
    platform_poll_input(&g_in);
    if (g_in.quit_requested) g_quit = 1;
}
static int clicked(void) { return (g_in.mouse_buttons & 1) && !(g_prev_buttons & 1); }
static int key(int k) { return k >= 0 && k < 256 && g_in.key_down[k]; }

static void draw_field(void) {
    battle_sort_sprites();
    battle_draw(g_fb);
}

/* ---- text boxes (59D4 messages and the report; layout approximate) ---- */
static int split_lines(const char *text, char lines[][96], int max) {
    int n = 0, k = 0;
    for (const char *p = text; ; p++) {
        if (*p == '\n' || *p == 0) {
            if (n < max) { lines[n][k] = 0; n++; }
            k = 0;
            if (!*p) break;
        } else if (k < 95 && n < max) lines[n][k++] = *p;
    }
    return n;
}
static void text_box(const char *title, const char *text, int extra_rows, int *bx, int *by, int *bw, int *bh) {
    char lines[24][96];
    int n = split_lines(text, lines, 24), wmax = title ? ui_text_width(title) : 0;
    for (int i = 0; i < n; i++) { int w = ui_text_width(lines[i]); if (w > wmax) wmax = w; }
    int rows = n + (title ? 1 : 0) + extra_rows;
    int w = wmax + 24, h = rows * 11 + 16;
    int x = (SCREEN_W - w) / 2, y = (SCREEN_H - h) / 2;
    ui_frame(g_fb, x, y, w, h);
    int ty = y + 8;
    if (title) { ui_text_centered(g_fb, x + w / 2, ty, title, 0); ty += 11; }
    for (int i = 0; i < n; i++, ty += 11) ui_text_centered(g_fb, x + w / 2, ty, lines[i], 0);
    *bx = x; *by = y; *bw = w; *bh = h;
}
static void wait_click_or_key(void) {
    if (g_headless) { shot("wait"); return; }
    for (;;) {
        present();
        if (g_quit || clicked()) return;
        for (int k = 1; k < 256; k++) if (g_in.key_down[k]) return;
        platform_sleep_ms(10);
    }
}
static void msg_hook(const char *s) {
    char t[160];
    snprintf(t, sizeof(t), "%s\nClick to continue", s);
    draw_field();
    int x, y, w, h; text_box(NULL, t, 0, &x, &y, &w, &h);
    wait_click_or_key();
}
/* E21A report: title + text as one block of centred lines, 11 px apart, centred on (159, 99) in the bevelled box;
   the numbers at fixed places: Rome's right-aligned to x 0x78, the enemy's from x 0xC4, rows y 0x8A / 0x95 */
void battle_report_draw(const char *text, unsigned rc, unsigned ec, unsigned rs, unsigned es) {
    char lines[24][96];
    int n = split_lines(text, lines, 24), wmax = 0;
    for (int i = 0; i < n; i++) { int w = ui_text_width(lines[i]); if (w > wmax) wmax = w; }
    int h = n * 11 - 3, top = 99 - h / 2, left = 159 - (wmax + 1) / 2;
    ui_frame(g_fb, left - 7, top - 8, wmax + 16, h + 17);
    for (int i = 0; i < n; i++) ui_text(g_fb, left + (wmax - ui_text_width(lines[i]) + 1) / 2, top - 1 + 11 * i, lines[i], 0);
    char num[16];
    snprintf(num, sizeof(num), "%u", rc); ui_text(g_fb, 0x78 - ui_text_width(num) - 1, 0x8A, num, 0);
    snprintf(num, sizeof(num), "%u", ec); ui_text(g_fb, 0xC4, 0x8A, num, 0);
    snprintf(num, sizeof(num), "%u", rs); ui_text(g_fb, 0x78 - ui_text_width(num) - 1, 0x95, num, 0);
    snprintf(num, sizeof(num), "%u", es); ui_text(g_fb, 0xC4, 0x95, num, 0);
}
static void report_hook(int won, const char *title, const char *text, unsigned rc, unsigned ec, unsigned rs, unsigned es) {
    (void)won; (void)title;
    draw_field();
    battle_report_draw(text, rc, ec, rs, es);
    wait_click_or_key();
}

/* ---- menus (54C8 with records DS:73B6 / 73C8) ---- */
static int g_pick[2];
static int menu_hook(int which) {
    menu_t m;
    ui_menu_load(&m, g_ovl0, which ? 0x73C8 : 0x73B6);
    if (which) {   /* the tactic menu's first four items are the player formation's tactics (76F6[C2AA][i]) */
        for (int i = 0; i < 4 && i < m.count; i++)
            snprintf(m.items[i].text, sizeof(m.items[i].text), "%s", dstr(W(0x76F6 + (B(0xC2AA) * 5 + i) * 10)));
        battle_sprites_build();   /* BB4D: the units stand on the field now */
    }
    for (;;) {
        if (which) draw_field(); else battle_draw(g_fb);
        if (!which) { setW(0xA928, 0); battle_draw(g_fb); }
        int hover = -1;
        for (int i = 0; i < m.count; i++)
            if (g_in.mouse_x >= m.x + m.items[i].x0 && g_in.mouse_x < m.x + m.w - 4 &&
                g_in.mouse_y >= m.y + m.items[i].y0 && g_in.mouse_y < m.y + m.items[i].y1) hover = i;
        ui_menu_draw(g_fb, &m, hover);
        if (g_headless) { shot(which ? "tactic" : "formation"); return g_pick[which]; }
        present();
        if (g_quit) return -1;
        if (key(27)) return -1;
        for (int i = 0; i < m.count; i++) {
            int hk = (unsigned char)m.items[i].hotkey;
            if (key(hk) || (hk >= 'A' && hk <= 'Z' && key(hk + 32))) return i;
        }
        if (clicked() && hover >= 0) return hover;
        platform_sleep_ms(10);
    }
}

/* ---- the orders mode (D0F2 -> D198 / D33A) ---- */
static void screen_to_grid(int mx, int my, int *gx, int *gy) {      /* C752 */
    int a = 2 * (my - 2) + (mx - 4), t = 2 * (my - 2) - (mx - 4);
    *gx = a / 16;
    if (t < 0) t -= 15;
    *gy = t / 16 + 0x14;
}
static int unit_at(int gx, int gy) {                                /* grid cell, else D2DD's 12 neighbours (DS:74BC) */
    for (int k = -1; k < 12; k++) {
        int x = gx + (k < 0 ? 0 : SB(0x74BC + 2 * k)), y = gy + (k < 0 ? 0 : SB(0x74BD + 2 * k));
        if (x < 0 || x >= 48 || y < 0 || y >= 48) continue;
        uint8_t c = B(W(0xC2AE) + x * 48 + y);
        if (c && c != 0xFF && c != 0xFE) return c;
    }
    return 0;
}
static void line(int x0, int y0, int x1, int y1, uint8_t c) {
    int dx = abs(x1 - x0), dy = -abs(y1 - y0), sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1, e = dx + dy;
    for (;;) {
        if (x0 >= 0 && x0 < SCREEN_W && y0 >= 0 && y0 < SCREEN_H) g_fb[y0 * SCREEN_W + x0] = c;
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * e;
        if (e2 >= dy) { e += dy; x0 += sx; }
        if (e2 <= dx) { e += dx; y0 += sy; }
    }
}
static void draw_path(int i) {                                     /* DA35 (approximate): the unit's order legs */
    uint16_t u = (uint16_t)(W(0xC292) + 0x12 * i);
    int x = SB(u + 2), y = SB(u + 3), sx, sy;
    battle_grid_to_screen(x, y, &sx, &sy);
    for (int q = 0; q < 3; q++) {
        uint8_t o = B(u + 6 + q);
        if (o == 0x80 || !(o & 0x3F)) break;
        int d = o >> 6, n = o & 0x3F;
        x += d == 0 ? n : d == 2 ? -n : 0; y += d == 1 ? n : d == 3 ? -n : 0;
        int tx, ty; battle_grid_to_screen(x, y, &tx, &ty);
        line(sx, sy, tx, ty, 15);
        sx = tx; sy = ty;
    }
}
static void draw_panel(int i) {                                    /* DC72 (approximate): what the unit is */
    uint16_t u = (uint16_t)(W(0xC292) + 0x12 * i);
    char l1[64], l2[64], num[16];
    uint8_t type = B(u + 0xA), side = B(u + 0xB);
    static const uint16_t tn[4] = {0x74F4, 0x74FD, 0x7508, 0x7512};
    if (type == 3) snprintf(l1, sizeof(l1), "%s", dstr(tn[3]));
    else { roman_numeral((unsigned)i, num); snprintf(l1, sizeof(l1), "%s%s%s", num, dstr(tn[type]), side ? dstr(0x751A) : " Cohort"); }
    if (B(u + 0xD) == 3) snprintf(l2, sizeof(l2), "%u men  %s", W(u), dstr(0x7520));
    else snprintf(l2, sizeof(l2), "%u men  %s", W(u), dstr(W(0x7486 + 2 * (B(u + 0xC) / 25 & 3))));
    ui_frame(g_fb, 2, 2, 118, 30);
    ui_text(g_fb, 8, 7, l1, 0); ui_text(g_fb, 8, 18, l2, 0);
}
static int button_at(int mx, int my) {                              /* D978: DS:748E rects, 0 Select, 1 Done, 2 Melee, 3 Retreat */
    for (int b = 0; b < 4; b++) {
        uint16_t r = (uint16_t)(0x748E + 8 * b);
        if (mx >= W(r) && mx <= W(r + 4) && my >= W(r + 2) && my <= W(r + 6)) return b;
    }
    return -1;
}
static void draw_buttons(void) {                                   /* D92D */
    for (int b = 0; b < 4; b++) {
        uint16_t r = (uint16_t)(0x748E + 8 * b);
        int x0 = W(r), y0 = W(r + 2), x1 = W(r + 4), y1 = W(r + 6);
        ui_frame(g_fb, x0, y0, x1 - x0 + 1, y1 - y0 + 1);
        ui_text_centered(g_fb, (x0 + x1) / 2, y0 + 6, dstr(W(0x74AE + 2 * b)), 0);   /* labels DS:74AE */
    }
}
static void orders_mode(void) {
    int sel = 0, dragging = 0;
    for (;;) {
        draw_field();
        draw_buttons();
        if (sel) { draw_panel(sel); draw_path(sel); }
        present();
        if (g_quit) return;
        int mx = g_in.mouse_x, my = g_in.mouse_y;
        if (dragging) {
            int gx, gy; screen_to_grid(mx, my, &gx, &gy);
            if (g_in.mouse_buttons & 1) { battle_cmd_target((uint8_t)gx, (uint8_t)gy); platform_sleep_ms(10); continue; }
            battle_cmd_end(); dragging = 0; sel = 0;
            continue;
        }
        int b = button_at(mx, my);
        int c = clicked();
        if (key(27) || key('d') || key(13) || (c && b == 1)) { if (sel) battle_cmd_end(); return; }
        if (key('s') || (c && b == 0)) {                 /* every Roman unit's orders */
            draw_field(); draw_buttons();
            for (int i = 1; i < W(0xC294); i++)
                if (B(W(0xC292) + 0x12 * i + 0xB) == 0 && B(W(0xC292) + 0x12 * i + 0xD) != 4) draw_path(i);
            wait_click_or_key();
            continue;
        }
        if (key('m') || (c && b == 2)) {
            if (sel) { battle_cmd_melee(); sel = 0; continue; }
            battle_order_legion_melee(); return;
        }
        if (key('r') || (c && b == 3)) {
            if (sel) { battle_cmd_retreat(); sel = 0; continue; }
            battle_order_legion_retreat(); return;
        }
        if (sel && key(' ')) { battle_cmd_hold(); battle_cmd_end(); sel = 0; continue; }
        if (c && b < 0) {
            int gx, gy; screen_to_grid(mx, my, &gx, &gy);
            int t = unit_at(gx, gy);
            if (sel && t == sel) {                       /* second click on the selected cohort: drag its destination */
                battle_cmd_begin(t, (uint8_t)gx, (uint8_t)gy); dragging = 1; continue;
            }
            sel = 0;
            if (!t) continue;
            if (B(W(0xC292) + 0x12 * t + 0xB) != 0 || !battle_cmd_begin(t, (uint8_t)gx, (uint8_t)gy)) {
                /* enemy, routed or out of the general's voice: the panel only */
                draw_field(); draw_buttons(); draw_panel(t); wait_click_or_key(); continue;
            }
            sel = t;                                     /* selected: panel + path; M / R / buttons act on it */
        }
        platform_sleep_ms(10);
    }
}

/* ---- the running battle: called by the simulation after each tick (D0F2's place) ---- */
static uint32_t g_tick_start;
static int g_shot_tick = -1;
static int tick_hook(int tick) {
    draw_field();
    if (g_headless) {
        if (tick == g_shot_tick) {
            shot("tick");
            draw_buttons(); shot("orders");
            for (int i = 1; i < W(0xC294); i++)
                if (B(W(0xC292) + 0x12 * i + 0xB) == 0 && B(W(0xC292) + 0x12 * i + 0xA) == 2) {
                    draw_field(); draw_buttons(); draw_panel(i); draw_path(i); shot("panel"); break;
                }
            msg_hook(dstr(0x74D4));
        }
        return 0;
    }
    present();
    uint32_t due = g_tick_start + 165;                             /* 12 ticks of the 72.8 Hz timer */
    int trigger = 0;
    for (;;) {
        if (g_quit) break;
        if (clicked() || key(' ') || key(13) || key('+') || key('=')) trigger = 1;
        if (platform_ticks_ms() >= due) break;
        platform_sleep_ms(5);
        g_prev_buttons = g_in.mouse_buttons;
        platform_poll_input(&g_in);
        if (g_in.quit_requested) g_quit = 1;
    }
    if (trigger && !g_quit) orders_mode();
    g_tick_start = platform_ticks_ms();
    return 0;
}
static void setup_hook(int Z) {
    if (assets_load("BATL.DIR", "BATL.DAT") != 0 || battle_gfx_load(Z) != 0)
        fprintf(stderr, "battle screen: BATL.DAT not found (run in the game folder)\n");
}

/* Headless picture check: the battle with menu choices pf/pt and no orders; saves the formation and tactic menus,
   the field at tick shot_tick with the orders buttons, a unit panel, a message box, and the report, as
   <prefix>_NN_<what>.bin (320x200 palette indices). */
uint16_t battle_play(int A, int L, int Z);
uint16_t battle_ui_shots(int A, int L, int Z, int pf, int pt, int shot_tick, const char *prefix) {
    g_headless = 1; g_shot_prefix = prefix; g_shot_n = 0; g_pick[0] = pf; g_pick[1] = pt; g_shot_tick = shot_tick;
    uint16_t r = battle_play(A, L, Z);
    g_headless = 0; g_shot_prefix = NULL;
    return r;
}

/* Fight battle (A, L, Z) on screen from the current DS. Returns the battle's result (0xFFFF = Rome won). */
uint16_t battle_play(int A, int L, int Z) {
    /* palette: CMAP.BIN in CENT.DAT (250 x 6-bit RGB) */
    size_t n = 0; uint8_t *cmap = NULL; int k;
    if (assets_load("CENT.DIR", "CENT.DAT") == 0 && (k = assets_find("CMAP.BIN")) >= 0) cmap = assets_decode(k, &n);
    for (int i = 0; i < 256; i++) g_pal[i].r = g_pal[i].g = g_pal[i].b = (uint8_t)i;
    for (size_t i = 0; cmap && i < 250 && 3 * i + 2 < n; i++) {
        g_pal[i].r = (uint8_t)(cmap[3 * i] << 2); g_pal[i].g = (uint8_t)(cmap[3 * i + 1] << 2); g_pal[i].b = (uint8_t)(cmap[3 * i + 2] << 2);
    }
    free(cmap);
    assets_load_luts("OVL0.OVL");
    assets_load_font("OVL0.OVL");
    FILE *f = fopen("OVL0.OVL", "rb");
    if (f) { fseek(f, 0, SEEK_END); long len = ftell(f); fseek(f, 0, SEEK_SET); g_ovl0 = malloc((size_t)len); if (fread(g_ovl0, 1, (size_t)len, f) != (size_t)len) {} fclose(f); }
    size_t tn = 0; uint8_t *tac = NULL;
    if (assets_load("BATL.DIR", "BATL.DAT") == 0 && (k = assets_find("TACTICS.LZW")) >= 0) tac = assets_decode(k, &tn);
    battle_menu_hook = menu_hook; battle_setup_hook = setup_hook; battle_message_hook = msg_hook;
    battle_report_hook = report_hook; battle_orders_hook = tick_hook;
    g_tick_start = platform_ticks_ms();
    uint16_t r = battle(A, L, Z, 0, 0, tac, tn);
    battle_menu_hook = NULL; battle_setup_hook = NULL; battle_message_hook = NULL; battle_report_hook = NULL; battle_orders_hook = NULL;
    free(tac); free(g_ovl0); g_ovl0 = NULL;
    return r;
}

/* picture check of the report: text from the DS (after a battle), over frame bg (or black) */
void battle_report_shot(int L, int won, unsigned rc, unsigned ec, unsigned rs, unsigned es, const uint8_t *bg, const char *out) {
    char text[600];
    assets_load_font("OVL0.OVL");
    battle_report_text(L, won, text, sizeof(text));
    if (bg) memcpy(g_fb, bg, sizeof(g_fb)); else memset(g_fb, 0, sizeof(g_fb));
    battle_report_draw(text, rc, ec, rs, es);
    FILE *f = fopen(out, "wb");
    if (f) { fwrite(g_fb, 1, sizeof(g_fb), f); fclose(f); }
}
