/* The land battle on screen: menus, the running battle, the orders mode, messages and the report.
 * Claude, 2026-11-09. The simulation (battle.c) and the drawing of the field and units (battle_gfx.c) are the
 * original's, verified; this file is the screen and input around them, translated from the code:
 *   orders mode  D0F2 / D198 / D978 / D9EF / D33A / D2DD (OVL_BATTLE), see docs/BATTLE_UI_NOTES.md
 *   drawing      D92D buttons, DA35 + DBEB order paths, DB8A/DBBF voice circle, DC72 unit panel
 *   EXE helpers  3134 box, 899C text, 8BE4 centred text, 8C58 width, 8C9F text rect, 59D4 message box,
 *                0524 3x3 dot, 97B6 line, 98BC/98DA/993E circle, C7B0 grid->screen, C752 screen->grid,
 *                691B/6973 save/restore (screen <-> work buffer), 359B/35DB/35F9 waits; see docs/TEXTBOX_NOTES.md
 *   report       E48B..E58B (OVL_BATTLE)
 * The menus (54C8) are still the port's approximation.
 *
 * Model of the original's two pictures in the orders mode: g_fb = the VGA screen (everything is drawn straight onto
 * it), g_work = the work buffer (seg 2687), used as the backup of the clean picture: 691B copies a rect of the screen
 * into it, 6973 copies it back.
 *
 * Input: the port always has a mouse, so the original's mouse paths ([00BA] != 0) are the ones translated. During a
 * tick, Space, Enter, '+' or a left or right mouse press open the orders mode (D0F2; the original's Insert cannot
 * reach the port through the SDL layer, see tick_hook) and the battle is frozen: buttons Select Roman
 * Unit / Melee / Retreat / Done (DS:748E rects, hotkeys S M R D, Esc = Done). Click a Roman cohort: its panel and
 * path; click it again and drag to give it a destination, or press M / R (or the Melee / Retreat buttons) for it.
 * Without a selected cohort, Melee = "Release The Legion!" and Retreat = "Run Away! Run Away!", both of which end the
 * orders mode. Clicking an enemy shows its panel while the button is held.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "state.h"
#include "assets.h"
#include "platform.h"
#include "ui.h"

static uint8_t g_fb[SCREEN_W * SCREEN_H];     /* the screen (VGA A000) */
static uint8_t g_work[SCREEN_W * SCREEN_H];   /* the work buffer (seg 2687): 691B / 6973 save area */
static rgb_t g_pal[256];
static uint8_t *g_ovl0;
static input_t g_in;
static int g_prev_buttons, g_quit;
static int g_b2;          /* [00B2]: mouse buttons down at the last poll (1 left, 2 right) */
static int g_c07f;        /* [C07F]: the last key */
static uint8_t g_col;     /* [9AFA]: line / dot colour */

static const char *dstr(uint16_t a) { return (const char *)&g_ds[a]; }
/* strcpy / strcat from the DS (9E32 / 9DF2), bounded */
static void ds_cpy(char *d, size_t n, uint16_t a) {
    size_t k = 0;
    while (k + 1 < n && g_ds[(uint16_t)(a + k)]) { d[k] = (char)g_ds[(uint16_t)(a + k)]; k++; }
    d[k] = 0;
}
static void ds_cat(char *d, size_t n, uint16_t a) { size_t l = strlen(d); if (l < n) ds_cpy(d + l, n - l, a); }

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
/* EXE 0692(b): INT 33h fn 5, press count of button b since the last call (b 0 = left, 1 = right); the port sees a press
   as a button bit new since the last poll (platform: bit 1 = left, bit 4 = right) */
static int pressed0692(int b) {
    int m = b ? 4 : 1;
    return (g_in.mouse_buttons & m) && !(g_prev_buttons & m);
}
static int key(int k) { return k >= 0 && k < 256 && g_in.key_down[k]; }

static void draw_field(void) {
    battle_sort_sprites();
    battle_draw(g_fb);
}

/* ---- input as the EXE sees it ---- */
static void poll0644(void) {                       /* EXE 0644: mouse state -> [B2] [B6] [B8] (the screen is live) */
    if (g_headless) { g_b2 = 0; return; }
    present();
    g_b2 = ((g_in.mouse_buttons & 1) ? 1 : 0) | ((g_in.mouse_buttons & 4) ? 2 : 0);
}
static int take_key(void) {                        /* EXE 03AA: next key or 0 (the port's key events of this poll) */
    for (int k = 1; k < 256; k++) if (g_in.key_down[k]) { g_in.key_down[k] = 0; return k; }
    return 0;
}
static void wait_release(void) {                   /* EXE 35DB: until no button is down */
    while (g_b2 && !g_quit) { if (!g_headless) platform_sleep_ms(10); poll0644(); }
}
static int wait_press(void) {                      /* EXE 359B: 1 / 2 = left / right button ([C07F] = 0), 0 = key in [C07F] */
    poll0644();
    for (;;) {
        if (g_quit || g_headless) { g_c07f = 0x1B; return 0; }
        if (g_b2) { g_c07f = 0; return (g_b2 & 1) ? 1 : 2; }
        int k = take_key();
        if (k) { g_c07f = k; return 0; }
        platform_sleep_ms(10);
        poll0644();
    }
}
static void wait_click(void) {                     /* EXE 35F9 */
    if (g_headless) { shot("wait"); return; }
    wait_release(); wait_press(); wait_release();
}

/* ---- pictures ---- */
static void save_rect(int x0, int y0, int x1, int y1, int restore) {   /* 691B (screen -> work) / 6973 (work -> screen) */
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > SCREEN_W - 1) x1 = SCREEN_W - 1;
    if (y1 > SCREEN_H - 1) y1 = SCREEN_H - 1;
    for (int y = y0; y <= y1 && x0 <= x1; y++) {
        uint8_t *s = &g_fb[y * SCREEN_W + x0], *w = &g_work[y * SCREEN_W + x0];
        if (restore) memcpy(s, w, (size_t)(x1 - x0 + 1)); else memcpy(w, s, (size_t)(x1 - x0 + 1));
    }
}
#define SAVE(x0, y0, x1, y1)    save_rect(x0, y0, x1, y1, 0)
#define RESTORE(x0, y0, x1, y1) save_rect(x0, y0, x1, y1, 1)

static void hspan(int xa, int y, int xb, uint8_t c) {                 /* EXE 7ECA (clipped to 0..319 x 0..199) */
    if (y < 0 || y >= SCREEN_H) return;
    if (xa > xb) { int t = xa; xa = xb; xb = t; }
    if (xa < 0) xa = 0;
    if (xb > SCREEN_W - 1) xb = SCREEN_W - 1;
    for (int x = xa; x <= xb; x++) g_fb[y * SCREEN_W + x] = c;
}
static void vspan(int x, int ya, int yb, uint8_t c) {
    if (x < 0 || x >= SCREEN_W) return;
    if (ya > yb) { int t = ya; ya = yb; yb = t; }
    if (ya < 0) ya = 0;
    if (yb > SCREEN_H - 1) yb = SCREEN_H - 1;
    for (int y = ya; y <= yb; y++) g_fb[y * SCREEN_W + x] = c;
}
/* EXE 3134 box(x0, y0, x1, y1, fill): corners inclusive, fill -1 = none; then the bevel in 8, 0, 15 (in this order) */
static void box3134(int x0, int y0, int x1, int y1, uint16_t fill) {
    if (fill != 0xFFFF) for (int y = y0; y <= y1; y++) hspan(x0, y, x1, (uint8_t)fill);   /* 7E74 */
    vspan(x0, y0, y1, 8); vspan(x0 + 1, y0 + 1, y1 - 1, 8); vspan(x1, y0, y1, 8); vspan(x1 - 1, y0 + 1, y1 - 1, 8);
    vspan(x0 + 3, y0 + 3, y1 - 3, 8); hspan(x0 + 3, y1 - 3, x1 - 3, 8); vspan(x1 - 3, y0 + 3, y1 - 3, 8);
    hspan(x0 + 3, y0 + 3, x1 - 3, 8);
    hspan(x0, y1, x1, 0); hspan(x0 + 1, y1 - 1, x1 - 1, 0);
    hspan(x0, y0, x1, 15); hspan(x0 + 1, y0 + 1, x1 - 1, 15);
}
/* EXE 8C58: advance width of the first line (stops at NUL or '\n'), the 1-px spacing after every glyph included;
   the character test is signed (bytes 0x80..0xFF add nothing) */
static int w8c58(const char *s) {
    int t = 0;
    for (; *s && *s != '\n'; s++) {
        int w, yo;
        if ((signed char)*s >= 0x20 && font_glyph((unsigned char)*s, &w, &yo)) t += w + 1;
    }
    return t;
}
/* EXE 8D56 one glyph: rows at y + yoff, set bits in colour, clear bits transparent */
static void glyph(int ch, int x, int y, uint8_t c) {
    int w, yoff;
    const uint8_t *g = font_glyph(ch, &w, &yoff);
    if (!g || !w) return;
    for (int r = 0; r < 8; r++) {
        int dy = y + yoff + r;
        if (dy < 0 || dy >= SCREEN_H) continue;
        for (int i = 0; i < 8; i++)
            if ((g[r] & (0x80 >> i)) && x + i >= 0 && x + i < SCREEN_W) g_fb[dy * SCREEN_W + x + i] = c;
    }
}
/* EXE 899C text(str, x, y) in colour [9B69] & 15: left-aligned, '\n' = back to x, 11 px down ([9B6A] background mode
   is not used by these screens) */
static void t899c(const char *s, int x, int y, uint8_t c) {
    int sx = x;
    for (; *s; s++) {
        if (*s == '\n') { sx = x; y += 11; }
        else if ((signed char)*s >= 0x20) {
            int w, yo;
            glyph((unsigned char)*s, sx, y, c);
            if (font_glyph((unsigned char)*s, &w, &yo)) sx += w + 1;
        }
    }
}
/* EXE 8BE4 centred_text(str, cx, y): every '\n' line at cx - (8C58(line) >> 1), 11 px apart */
static void t8be4(const char *s, int cx, int y, uint8_t c) {
    char line[256];
    while (s) {
        const char *nl = strchr(s, '\n');
        size_t n = nl ? (size_t)(nl - s) : strlen(s);
        if (n > sizeof(line) - 1) n = sizeof(line) - 1;
        memcpy(line, s, n); line[n] = 0;
        t899c(line, cx - (int)((unsigned)w8c58(line) >> 1), y, c);
        y += 11;
        s = nl ? nl + 1 : NULL;
    }
}
/* EXE 8C9F text_rect(str, minw, lines0, cx, cy): widest line (at least minw), lines0 + number of lines (a leading
   '\n' is skipped, a trailing one counts an empty line); r = {cx - w/2, cy - 11n/2, cx + w/2, cy + 11n/2} */
static void r8c9f(const char *s, int minw, int lines0, int cx, int cy, int r[4]) {
    int n = lines0, mw = minw;
    if (*s) {
        const char *p = s;
        do {
            if (*p == '\n') p++;
            if (w8c58(p) > mw) mw = w8c58(p);
            n++;
            p = strchr(p, '\n');
        } while (p);
    }
    int h = (n * 11) / 2, hw = mw / 2;
    r[0] = cx - hw; r[1] = cy - h; r[2] = cx + hw; r[3] = cy + h;
}

/* DS:[3E40], the click line of 59D4: the static OVL0 image points at DS:3E16 "Hit any key to continue", the game with a
   mouse runs with DS:3E2E "Click to continue" (t10_in.bin, b11_entry.bin). The port always has a mouse, so it is 3E2E
   whatever the DS was built from (the DOSBox snapshots and a DS from state_init alike). */
static const char *click_text(void) {
    if (memcmp(&g_ds[0x3E2E], "Click to continue", 18) == 0) return dstr(0x3E2E);
    return "Click to continue";
}
/* DS:[6346], the box fill of 3134 on these screens: 7 in the static OVL0 image, set to 1Ah by the start-up's VGA branch
   (resident overlay 0123..014C, [9B26] == 2) and 26 in every snapshot; the port is always VGA, so 1Ah whatever the DS
   was built from */
static uint16_t fill6346(void) { return 0x1A; }
/* EXE 59D4 message_box(str): centred on (159, 99), at least as wide as "Click to continue" ([3E40]), one extra line
   for it; the screen under the box is saved into the work buffer (691B) and put back after a click or key (6973). */
static void message_box(const char *s) {
    if (!s) return;
    const char *click = click_text();
    int r[4]; r8c9f(s, w8c58(click), 1, 159, 99, r);
    int X0 = r[0] - 7, Y0 = r[1] - 7, X1 = r[2] + 7, Y1 = r[3] + 7;
    SAVE(X0, Y0, X1, Y1);
    box3134(X0, Y0, X1, Y1, fill6346());
    t8be4(s, (X0 + X1) / 2, Y0 + 7, 0);
    t8be4(click, 159, Y1 - 15, 15);
    wait_click();
    RESTORE(X0, Y0, X1, Y1);
    if (!g_headless) present();
}
static void msg_hook(const char *s) { message_box(s); }

/* E48B..E58B the battle report: title + text as one block centred on (159, 99) (8C9F with minw = 8C58 of the string
   [7AD4] points to, no extra line), box 3134 7 px around it, text in black by 8BE4; Rome's numbers right-aligned to
   x 0x78 (8AC8), the enemy's from x 0xC4 (8A9E), rows y 0x8A / 0x95. Stays on the screen (no 691B / 6973). The casualty
   totals DS:C082 / C086 are added in battle.c (result). */
void battle_report_draw(const char *text, unsigned rc, unsigned ec, unsigned rs, unsigned es) {
    int r[4]; r8c9f(text, w8c58(dstr(W(0x7AD4))), 0, 159, 99, r);
    int X0 = r[0] - 7, Y0 = r[1] - 7, X1 = r[2] + 7, Y1 = r[3] + 7;
    box3134(X0, Y0, X1, Y1, fill6346());
    t8be4(text, (X0 + X1) / 2, Y0 + 7, 0);
    char num[16];
    snprintf(num, sizeof(num), "%u", rc & 0xFFFF); t899c(num, 0x78 - w8c58(num), 0x8A, 0);
    snprintf(num, sizeof(num), "%u", ec & 0xFFFF); t899c(num, 0xC4, 0x8A, 0);
    snprintf(num, sizeof(num), "%u", rs & 0xFFFF); t899c(num, 0x78 - w8c58(num), 0x95, 0);
    snprintf(num, sizeof(num), "%u", es & 0xFFFF); t899c(num, 0xC4, 0x95, 0);
}
static void report_hook(int won, const char *title, const char *text, unsigned rc, unsigned ec, unsigned rs, unsigned es) {
    (void)won; (void)title;
    draw_field();
    battle_report_draw(text, rc, ec, rs, es);
    wait_click();
}

/* ---- menus (5614 + 54C8 with records DS:73B6 / 73C8, ui.c: pixel-exact drawing, 54C8 polling) ---- */
static int g_pick[2];
static int menu_hook(int which) {
    menu_t m;
    ui_menu_load(&m, g_ovl0, which ? 0x73C8 : 0x73B6);
    if (which) {   /* the tactic menu's first four items are the player formation's tactics (76F6[C2AA][i]) */
        for (int i = 0; i < 4 && i < m.count; i++)
            snprintf(m.items[i].text, sizeof(m.items[i].text), "%s", dstr(W(0x76F6 + (B(0xC2AA) * 5 + i) * 10)));
        battle_sprites_build();   /* BB4D: the units stand on the field now */
    }
    if (!which) setW(0xA928, 0);              /* the formation menu shows the bare field */
    int held = 1;                             /* 54C8 first waits for the buttons to be released */
    for (;;) {
        if (which) draw_field(); else battle_draw(g_fb);
        ui_menu_draw(g_fb, &m, -1);           /* no hover highlight in the original */
        if (g_headless) { shot(which ? "tactic" : "formation"); return g_pick[which]; }
        present();
        if (g_quit) return -1;
        int r = ui_menu_run(&m, &g_in, &held);
        if (r == -1) return -1;               /* Esc */
        if (r >= 0) {                         /* 585B: the chosen item in colour 15, then the release wait */
            ui_menu_highlight(g_fb, &m, r, 1);
            do { present(); platform_sleep_ms(10); } while (!g_quit && (g_in.mouse_buttons & 5));
            return r;
        }
        platform_sleep_ms(10);
    }
}

/* ---- the orders mode: drawing ---- */
static uint16_t unit(int i) { return (uint16_t)(W(0xC292) + 0x12 * i); }

static void c7b0(int8_t x, int8_t y, int *sx, int *sy) {           /* overlay C7B0: grid cell -> screen */
    *sx = (int16_t)((x - y) * 8 + 0xA4); *sy = (int16_t)((x + y) * 4 - 0x4A);
}
static void c752(int mx, int my, int8_t *gx, int8_t *gy) {         /* overlay C752: screen -> grid cell (clicks) */
    int a = 2 * (my - 2) + (mx - 4), b = 2 * (my - 2) - (mx - 4);
    *gx = (int8_t)(a / 16);
    if (b < 0) b -= 15;
    *gy = (int8_t)(b / 16 + 0x14);
}
static void dot0524(int x, int y) {                                 /* EXE 0524: 3x3 dot in colour [9AFA] */
    for (int yy = y - 1; yy <= y + 1; yy++) hspan(x - 1, yy, x + 1, g_col);
}
/* EXE 97B6(x1, y1, x2, y2, plot): Bresenham, both end points plotted; dx == dy takes the y-major branch */
static void line97b6(int x1, int y1, int x2, int y2) {
    int sx = 1, dx = x2 - x1, sy = 1, dy = y2 - y1, x = x1, y = y1;
    if (dx < 0) { sx = -1; dx = -dx; }
    if (dy < 0) { sy = -1; dy = -dy; }
    if (dx == 0) { for (int c = dy; c; c--) { dot0524(x, y); y += sy; } }
    else if (dy == 0) { for (int c = dx; c; c--) { dot0524(x, y); x += sx; } }
    else if (dx > dy) {
        int i1 = 2 * dy, e = 2 * dy - dx, i2 = 2 * dy - 2 * dx;
        for (int c = dx; c; c--) { dot0524(x, y); x += sx; if (e < 0) e += i1; else { y += sy; e += i2; } }
    } else {
        int i1 = 2 * dx, e = 2 * dx - dy, i2 = 2 * dx - 2 * dy;
        for (int c = dy; c; c--) { dot0524(x, y); y += sy; if (e < 0) e += i1; else { x += sx; e += i2; } }
    }
    dot0524(x, y);
}
/* overlay DBEB: centre of a unit's footprint on the screen, 4 px above its middle (footprints DS:6AE8 / 6AEC by
   type, swapped for facing 0 and 2) */
static void dbeb(int8_t cx, int8_t cy, uint8_t facing, uint8_t type, int *ox, int *oy) {
    uint8_t a = B(0x6AE8 + type), b = B(0x6AEC + type);
    if (facing == 2 || facing == 0) { uint8_t t = a; a = b; b = t; }
    int x1, y1, x2, y2;
    c7b0(cx, cy, &x1, &y1);
    c7b0((int8_t)(cx + a), (int8_t)(cy + b), &x2, &y2);
    *ox = (int16_t)(x1 + x2) / 2;
    *oy = (int16_t)(y1 + y2 - 8) / 2;
}
/* EXE 98BC / 98DA / 993E: midpoint circle of radius r around (cx, cy), the 8 symmetric points of each step collected
   and handed to the callback in reverse order; here the callback is DBBF: grid cell -> C7B0 -> 3x3 dot */
static void dbbf(int x, int y) { int sx, sy; c7b0((int8_t)x, (int8_t)y, &sx, &sy); dot0524(sx, sy); }
static void octant993e(int cx, int cy, int a, int b) {
    int px[8], py[8], n = 0;
    for (int pass = 0; pass < 2; pass++) {
        px[n] = cx + a; py[n] = cy + b; n++;
        if (b) {
            px[n] = cx + a; py[n] = cy - b; n++;
            if (a) { px[n] = cx - a; py[n] = cy - b; n++; px[n] = cx - a; py[n] = cy + b; n++; }
        } else if (a) { px[n] = cx - a; py[n] = cy - b; n++; }
        int t = a; a = b; b = t;
    }
    for (int k = n - 1; k >= 0; k--) dbbf(px[k], py[k]);
}
static void circle98da(int cx, int cy, int r) {
    int x = 0, y = r, d = 3 - 2 * r;
    do {
        octant993e(cx, cy, x, y);
        if (d < 0) { d += 4 * x + 5; x++; }
        else { d += 4 * (x - y) + 10; y--; x++; }
    } while (x < y);
    if (x == y) octant993e(cx, cy, x, y);
}
static void db8a(int i) {                                           /* overlay DB8A: the general's voice circle */
    uint16_t u = unit(i);
    circle98da(SB(u + 2), SB(u + 3), B(0x1A1F + 6 * B(0xC28C)));
}
/* overlay DA35(i): unit i's order path in colour 1: the footprint centre now and after each of the three legs
   (+6..+8; a leg 0x80 = a dot at the start, its point left out, the walk goes on), joined by lines of 3x3 dots;
   the general also gets his voice circle. Units in state 4 draw nothing. */
static void draw_path(int i) {
    uint16_t u = unit(i);
    if (B(u + 0xD) == 4) return;
    g_col = 1;
    uint8_t type = B(u + 0xA), fac = B(u + 0xE);
    int px[4], py[4];
    for (int k = 0; k < 4; k++) px[k] = py[k] = -1;
    int8_t cx = SB(u + 2), cy = SB(u + 3);
    dbeb(cx, cy, fac, type, &px[0], &py[0]);
    for (int k = 0; k < 3; k++) {
        uint8_t o = B(u + 6 + k);
        if (o == 0x80) { dot0524(px[0], py[0]); continue; }
        fac = (uint8_t)(((int8_t)o >> 6) & 3);
        uint8_t n = o & 0x3F;
        switch (fac) {
        case 0: cx = (int8_t)(cx + n); break;
        case 1: cy = (int8_t)(cy + n); break;
        case 2: cx = (int8_t)(cx - n); break;
        case 3: cy = (int8_t)(cy - n); break;
        }
        dbeb(cx, cy, fac, type, &px[k + 1], &py[k + 1]);
    }
    for (int k = 1; k <= 3; k++)
        if (px[k] != -1 && py[k] != -1 && px[k - 1] != -1 && py[k - 1] != -1)
            line97b6(px[k - 1], py[k - 1], px[k], py[k]);
    if (type == 3) db8a(i);
}
/* overlay DC72(i): the unit's panel, on the right half if the unit stands left of x 159, else on the left.
   Cohorts / enemy units: box (2|247, 24)-(+70, 64), "<numeral> Cohort|Unit", "<men> <type>", the morale word in its
   colour (DS:7486 / 7544 by morale / 25) or "Panicked" in 4. Generals: "General" (enemy) or "<name>\nCharisma:N\nVoice:N",
   centred in a box sized by 8C9F and anchored at x 317 or 2. Nothing for a unit without men. */
static void draw_panel(int i) {
    uint16_t u = unit(i);
    char buf[160], num[16];
    if (W(u) == 0) return;
    int sx, sy; c7b0(SB(u + 2), SB(u + 3), &sx, &sy);
    int x1 = 2;
    if (sx < 0x9F) x1 = 0xF7;
    int x2 = x1 + 0x46;
    if (B(u + 0xA) != 3) {
        box3134(x1, 0x18, x2, 0x40, fill6346());
        roman_numeral((unsigned)i, buf);                                     /* 6738 of the unit index */
        ds_cat(buf, sizeof(buf), B(u + 0xB) == 1 ? 0x751A : W(0x74BA));      /* " Unit" / " Cohort" */
        t899c(buf, x1 + 5, 0x1D, 15);
        snprintf(buf, sizeof(buf), "%d", SW(u));                             /* 9FDC itoa(men, 10) */
        ds_cat(buf, sizeof(buf), W(0x753C + 2 * B(u + 0xA)));               /* type name */
        t899c(buf, x1 + 5, 11 + 0x1D, 15);
        if (B(u + 0xD) == 3) { t899c(dstr(0x7520), x1 + 5, 2 * 11 + 0x1D, 4); return; }   /* "Panicked" */
        int m = (int8_t)B(u + 0xC) / 25;                                    /* idiv: toward 0 */
        ds_cpy(buf, sizeof(buf), W(0x7486 + 2 * m));
        t899c(buf, x1 + 5, 2 * 11 + 0x1D, (uint8_t)(W(0x7544 + 2 * m) & 15));
        return;
    }
    if (B(u + 0xB) == 1) ds_cpy(buf, sizeof(buf), W(0x7542));                /* "General" */
    else {
        uint16_t c = (uint16_t)(6 * B(0xC28C));
        ds_cpy(buf, sizeof(buf), W(0x1A1C + c));
        ds_cat(buf, sizeof(buf), 0x7529);                                   /* "\nCharisma:" */
        snprintf(num, sizeof(num), "%d", B(0x1A1E + c)); strncat(buf, num, sizeof(buf) - strlen(buf) - 1);
        ds_cat(buf, sizeof(buf), 0x7534);                                   /* "\nVoice:" */
        snprintf(num, sizeof(num), "%d", B(0x1A1F + c)); strncat(buf, num, sizeof(buf) - strlen(buf) - 1);
    }
    int r[4]; r8c9f(buf, 0, 0, 0, 0, r);
    int w = (r[2] + 7) - (r[0] - 7), h = (r[3] + 7) - (r[1] - 7);
    if (sx < 0x9F) { x2 = 0x13D; x1 = 0x13D - w; } else { x1 = 2; x2 = w + 2; }
    box3134(x1, 0x18, x2, h + 0x18, fill6346());
    t8be4(buf, (x1 + x2) / 2, 0x18 + 7, 15);
}
/* overlay D92D: the four buttons (rects DS:748E, labels DS:74AE) in white, left-aligned 5 px inside */
static void draw_buttons(void) {
    for (int b = 0; b < 4; b++) {
        uint16_t r = (uint16_t)(0x748E + 8 * b);
        box3134(SW(r), SW(r + 2), SW(r + 4), SW(r + 6), fill6346());
        t899c(dstr(W(0x74AE + 2 * b)), SW(r) + 5, SW(r + 2) + 5, 15);
    }
}

/* ---- the orders mode: the flow ---- */
static uint8_t grid_at(int8_t x, int8_t y) {                        /* the field grid [C2AE], 48 x 48, no bounds check */
    return B((uint16_t)(W(0xC2AE) + (int16_t)(x * 48 + y)));
}
static uint8_t d2dd(int8_t x, int8_t y) {                           /* overlay D2DD: the 12 neighbours (DS:74BC) */
    uint8_t c = 0;
    for (uint16_t p = 0x74BC; p < 0x74D4; p += 2) {
        c = grid_at((int8_t)(x + SB(p)), (int8_t)(y + SB(p + 1)));
        if (c != 0 && c != 0xFF && c != 0xFE) break;
    }
    return c;
}
static int d978(void) {                                             /* overlay D978: key / click -> 0 S, 1 D, 2 M, 3 R, 4 field */
    switch (toupper(g_c07f)) { case 'S': return 0; case 'D': return 1; case 'M': return 2; case 'R': return 3; }
    if (g_b2 == 0) return 4;                                        /* mouse present ([BA] != 0), no button down */
    for (int b = 0; b < 4; b++) {                                   /* EXE 8238: inclusive rect test */
        uint16_t r = (uint16_t)(0x748E + 8 * b);
        if (g_in.mouse_x >= SW(r) && g_in.mouse_x <= SW(r + 4) && g_in.mouse_y >= SW(r + 2) && g_in.mouse_y <= SW(r + 6))
            return b;
    }
    return 4;
}
static void d9ef(void) {                                            /* overlay D9EF: every Roman unit's orders */
    SAVE(0, 0, 319, 199);
    for (int i = 1; i < W(0xC294); i++) if (B(unit(i) + 0xB) == 0) draw_path(i);
    wait_click();
    RESTORE(0, 0, 319, 199);
}
static void d33a(void) {                                            /* overlay D33A: a click on the field */
    int8_t gx, gy; c752(g_in.mouse_x, g_in.mouse_y, &gx, &gy);
    uint8_t t = grid_at(gx, gy);
    if (!t) t = d2dd(gx, gy);
    if (t == 0 || t == 0xFF || t == 0xFE) return;
    uint16_t u = unit(t);
    if (B(u + 0xB) != 0) { draw_panel(t); wait_release(); return; }   /* enemy: its panel while the button is held */
    SAVE(0, 0, 319, 199);                                           /* backup with the buttons */
    draw_panel(t); draw_path(t);
    int inv = battle_in_voice(t);
    wait_release();
    if (B(u + 0xD) == 3 || !inv) { wait_release(); RESTORE(0, 0, 319, 199); return; }
    g_c07f = 0;
    for (int got = 0; !got; ) {                                     /* the next click or M R D S; Esc leaves */
        if (wait_press()) { g_c07f = 0; got = 1; }
        else if (g_c07f == 0x1B) { g_c07f = 0; return; }
        else { g_c07f = toupper(g_c07f); if (g_c07f == 'M' || g_c07f == 'R' || g_c07f == 'D' || g_c07f == 'S') got = 1; }
    }
    int8_t cx, cy; c752(g_in.mouse_x, g_in.mouse_y, &cx, &cy);
    uint8_t t2 = grid_at(cx, cy);
    if (t2 != t) t2 = d2dd(cx, cy);
    battle_cmd_begin(t, (uint8_t)cx, (uint8_t)cy);                  /* keeps the first leg's direction (D4B4) */
    if (t2 == t && g_c07f == 0) {                                   /* the same unit clicked: drag its destination */
        while (g_b2 && !g_quit) {
            poll0644();
            int8_t nx, ny; c752(g_in.mouse_x, g_in.mouse_y, &nx, &ny);
            if (nx != cx || ny != cy) {
                RESTORE(0, 0, 319, 199);                            /* the panel and the old path disappear */
                battle_cmd_target((uint8_t)nx, (uint8_t)ny);        /* +6 = CBEE(t, cell), +7 = +8 = 0x80 */
                draw_path(t);
                cx = nx; cy = ny;
            }
            if (!g_headless) platform_sleep_ms(10);
        }
        battle_cmd_end();                                           /* moved: +4 = FFFF, state 1 */
        RESTORE(0, 0, 319, 199);
        return;
    }
    RESTORE(0, 0, 319, 199);
    switch (d978()) {
    case 2: battle_cmd_melee(); break;                              /* D7E8 "Release Cohort <n>" */
    case 3: battle_cmd_retreat(); break;                            /* D77C "Run Away!" */
    default: break;
    }
}
static int d198(void) {                                             /* overlay D198: one round of the orders menu */
    int got = 0, ret = 0;
    poll0644();
    RESTORE(0, 0, 319, 199);
    draw_buttons();
    while (!got) {
        if (wait_press()) { g_c07f = 0; got = 1; }
        else if (g_c07f == 0x1B) { g_c07f = 0; return -1; }
        else { g_c07f = toupper(g_c07f); if (g_c07f == 'M' || g_c07f == 'R' || g_c07f == 'D' || g_c07f == 'S') got = 1; }
    }
    switch (d978()) {
    case 0: d9ef(); break;
    case 1: ret = -1; break;
    case 2: battle_order_legion_melee(); ret = -1; break;           /* D21E "Release The Legion!" */
    case 3: battle_order_legion_retreat(); ret = -1; break;         /* D299 "Run Away!\nRun Away!" */
    case 4: d33a(); break;
    }
    wait_release();
    return ret;
}
static void orders_mode(void) {                                     /* overlay D0F2, the triggered branch */
    draw_field();                                                   /* 6837 + BB7E + BA72: field and units on the screen */
    SAVE(0, 0, 319, 199);
    while (!d198()) if (g_quit) break;
    RESTORE(0, 0, 319, 199);                                        /* 6973 + 6887: the work buffer shown */
    present();
}

/* ---- the running battle: called by the simulation after each tick (D0F2's place) ---- */
static uint32_t g_tick_start;
static int g_shot_tick = -1;
static void shot_orders_screens(void) {                             /* headless: what the orders mode shows */
    draw_field(); shot("tick");
    SAVE(0, 0, 319, 199);
    RESTORE(0, 0, 319, 199); draw_buttons(); shot("orders");                        /* D198 */
    int coh = 0, gen = 0, enemy = 0;
    for (int i = 1; i < W(0xC294); i++) {
        uint16_t u = unit(i);
        if (B(u + 0xB) == 0 && B(u + 0xA) == 2 && !coh) coh = i;
        if (B(u + 0xB) == 0 && B(u + 0xA) == 3 && !gen) gen = i;
        if (B(u + 0xB) == 1 && !enemy) enemy = i;
    }
    SAVE(0, 0, 319, 199);                                                           /* backup with the buttons */
    if (coh) { draw_panel(coh); draw_path(coh); shot("panel"); RESTORE(0, 0, 319, 199); }   /* D33A on a cohort */
    if (gen) { draw_panel(gen); draw_path(gen); shot("general"); RESTORE(0, 0, 319, 199); } /* D33A on the general */
    if (enemy) { draw_panel(enemy); shot("enemy"); RESTORE(0, 0, 319, 199); }          /* D33A on an enemy unit */
    for (int i = 1; i < W(0xC294); i++) if (B(unit(i) + 0xB) == 0) draw_path(i);
    shot("select"); RESTORE(0, 0, 319, 199);                                        /* D9EF */
    message_box(dstr(0x74D4));                                                      /* D299's message */
}
static int (*g_ext_orders)(int tick);      /* battle_ui_script: the script's orders, given where D0F2 polls */
static int tick_hook(int tick) {
    draw_field();
    if (g_headless) {
        if (tick == g_shot_tick) shot_orders_screens();
        if (g_ext_orders) g_ext_orders(tick);
        return 0;
    }
    present();
    uint32_t due = g_tick_start + 165;                             /* 12 ticks of the 72.8 Hz timer */
    int trigger = 0;
    for (;;) {
        if (g_quit) break;
        /* D104..D120: 0692(0), 0692(1): a left or right press; D123..D13D: 03AA's key in {20h, 10h (Insert), '+', 0Dh}.
           The SDL layer reports keysyms of the unshifted key and no modifier state, so the US '+' (Shift + '=') arrives
           as '='; '=' is accepted for it (plain '=' therefore triggers too, which the original does not). Insert
           (03AA: scan code 52h -> 10h) never reaches key_down (sdl2_platform.c keeps only keysyms < 256): not ported. */
        if (pressed0692(0) || pressed0692(1) || key(' ') || key(0x10) || key('+') || key(13) || key('=')) trigger = 1;
        if (platform_ticks_ms() >= due) break;
        platform_sleep_ms(5);
        g_prev_buttons = g_in.mouse_buttons;
        platform_poll_input(&g_in);
        if (g_in.quit_requested) g_quit = 1;
    }
    if (trigger && !g_quit) { memset(g_in.key_down, 0, sizeof(g_in.key_down)); orders_mode(); }   /* 04ED: flush keys */
    g_tick_start = platform_ticks_ms();
    return 0;
}
static void setup_hook(int Z) {
    if (assets_load("BATL.DIR", "BATL.DAT") != 0 || battle_gfx_load(Z) != 0)
        fprintf(stderr, "battle screen: BATL.DAT not found (run in the game folder)\n");
}

/* Headless picture check: the battle with menu choices pf/pt and no orders; saves the formation and tactic menus,
   at tick shot_tick the field, the orders buttons, a cohort's panel + path, the general's panel + path + voice circle,
   an enemy unit's panel, all Roman paths (Select), a message box, and the report, as <prefix>_NN_<what>.bin
   (320x200 palette indices). */
uint16_t battle_play(int A, int L, int Z);
uint16_t battle_ui_shots(int A, int L, int Z, int pf, int pt, int shot_tick, const char *prefix) {
    g_headless = 1; g_shot_prefix = prefix; g_shot_n = 0; g_pick[0] = pf; g_pick[1] = pt; g_shot_tick = shot_tick;
    uint16_t r = battle_play(A, L, Z);
    g_headless = 0; g_shot_prefix = NULL;
    return r;
}

/* The battle of a scripted (headless) game run (battle_auto with battle_ui_attach(1) and a script): the same screens as
   battle_play, drawn but not shown, with the menu choices pf / pt and the orders given by `orders` after each tick
   (main.c's --orders replayer fed with the script's "orders" lines; NULL = none). */
uint16_t battle_ui_script(int A, int L, int Z, int pf, int pt, int (*orders)(int tick));
uint16_t battle_ui_script(int A, int L, int Z, int pf, int pt, int (*orders)(int tick)) {
    g_headless = 1; g_shot_prefix = NULL; g_pick[0] = pf; g_pick[1] = pt; g_shot_tick = -1; g_ext_orders = orders;
    uint16_t r = battle_play(A, L, Z);
    g_headless = 0; g_ext_orders = NULL;
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
    int (*saved_orders)(int) = battle_orders_hook;      /* a caller's hook (main.c --orders) is put back afterwards */
    battle_menu_hook = menu_hook; battle_setup_hook = setup_hook; battle_message_hook = msg_hook;
    battle_report_hook = report_hook; battle_orders_hook = tick_hook;
    g_tick_start = platform_ticks_ms();
    uint16_t r = battle(A, L, Z, 0, 0, tac, tn);
    battle_menu_hook = NULL; battle_setup_hook = NULL; battle_message_hook = NULL; battle_report_hook = NULL;
    battle_orders_hook = saved_orders;
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
