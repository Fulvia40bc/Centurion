/* The screen, the input and the shared boxes of the EXE (segment 0823), Claude 2026-11-09.
 *
 *   input    0x0644 mouse poll, 0x066A pointer warp, 0x03AA key read, 0x3603 key (mouse path), 0x359B wait for a
 *            click or key, 0x35DB wait for release, 0x35F9 wait click  (disasm/EXE_RUNTIME.clean.asm; MENU_NOTES 4,
 *            TEXTBOX_NOTES 2 + corrections 2, GAMELOOP_NOTES 2)
 *   boxes    0x59D4 message box, 0x5C20 yes / no (record DS:3E5A), 0x54C8 menu run around ui.c's 0x5614 / 0x585B
 *
 * The pictures are g_scr (VGA A000: everything is drawn straight onto it, as in the original) and g_wrk (the work
 * buffer at 2687:0000, which 0x691B / 0x6973 use as the save area at the same coordinates).
 *
 * Input sources: the window (sdl2_platform.c) or a script (headless tests, screen_script()). The port always has a
 * mouse: W(0x00BA) = FFFF in every snapshot, and the keyboard-only paths (the keyboard cursor of 0x3603, 0x5901) are
 * not ported. Not ported either: the mouse cursor sprite 0x1BE6 / 0x1D55 / 0x1D8A (the window shows the system
 * pointer) and the music toggle that the INT 9 hook does on Ctrl+S (03FF; the key itself is swallowed as 0x03AA does).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "screen.h"
#include "state.h"
#include "assets.h"
#include "platform.h"

uint8_t g_scr[64000];
uint8_t g_wrk[64000];
int g_battle_ui_attached;

static rgb_t g_pal[256];
static uint8_t *g_ovl0;
static int g_headless = 1, g_window, g_quit;

/* window input: the last platform poll, and the BIOS key buffer (15 keys, as INT 16h's) */
static input_t g_in;
static int g_wx, g_wy, g_wb;                 /* pointer x / y, buttons in [B2] form (bit 0 left, bit 1 right) */
#define KBUF 15
static uint8_t g_kbuf[KBUF];
static int g_kn;

/* script */
enum { EV_CLICK, EV_RCLICK, EV_KEY, EV_SHOT, EV_BATTLE, EV_ORDERS, EV_END };
typedef struct { int kind, a, b, used; char s[256]; } sev_t;
static sev_t *g_ev;
static int g_nev, g_pos, g_scripted;
static char g_shotdir[512];
static int g_sx, g_sy, g_sb;                 /* the script's mouse: x, y, buttons ([B2] form) */
static char g_orders[1024];

static int cdiv2(int v) { return v / 2; }    /* cwd; sub ax, dx; sar ax, 1 = C division, toward 0 */

/* ---- opening ---- */
static uint8_t *read_file(const char *path, long *len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *p = n > 0 ? malloc((size_t)n) : NULL;
    if (p && fread(p, 1, (size_t)n, f) != (size_t)n) { free(p); p = NULL; }
    fclose(f);
    if (len) *len = n;
    return p;
}

/* palette CMAP.BIN (CENT.DAT, 250 x 6-bit RGB), CBM LUTs and the font (OVL0.OVL), the OVL0 bytes; a window unless headless.
   Run in the game folder. Returns 0, or -1 when OVL0.OVL / the font / the window cannot be had. */
int screen_open(int headless) {
    size_t n = 0; uint8_t *cmap = NULL; int k;
    if (assets_load("CENT.DIR", "CENT.DAT") == 0 && (k = assets_find("CMAP.BIN")) >= 0) cmap = assets_decode(k, &n);
    for (int i = 0; i < 256; i++) g_pal[i].r = g_pal[i].g = g_pal[i].b = (uint8_t)i;
    for (size_t i = 0; cmap && i < 250 && 3 * i + 2 < n; i++) {
        g_pal[i].r = (uint8_t)(cmap[3 * i] << 2); g_pal[i].g = (uint8_t)(cmap[3 * i + 1] << 2); g_pal[i].b = (uint8_t)(cmap[3 * i + 2] << 2);
    }
    if (!cmap) fprintf(stderr, "screen: CMAP.BIN not found in CENT.DAT (grey palette)\n");
    free(cmap);
    assets_load_luts("OVL0.OVL");
    if (assets_load_font("OVL0.OVL") != 0) { fprintf(stderr, "screen: no font in OVL0.OVL (run in the game folder)\n"); return -1; }
    free(g_ovl0);
    if (!(g_ovl0 = read_file("OVL0.OVL", NULL))) { fprintf(stderr, "screen: cannot read OVL0.OVL\n"); return -1; }
    g_headless = headless; g_window = 0; g_quit = 0; g_kn = 0;
    memset(g_scr, 0, sizeof(g_scr)); memset(g_wrk, 0, sizeof(g_wrk));
    if (!headless) {
        if (platform_init() != 0) { fprintf(stderr, "screen: no window\n"); return -1; }
        g_window = 1;
    }
    return 0;
}

const uint8_t *screen_ovl0(void) { return g_ovl0; }
const rgb_t *screen_palette(void) { return g_pal; }
int screen_headless(void) { return g_headless; }
int screen_scripted(void) { return g_scripted; }
int screen_quit(void) { return g_quit; }
void battle_ui_attach(int on) { g_battle_ui_attached = on; }

/* headless: g_scr to <shotdir>/<name>.bin (320x200 palette indices) */
void screen_shot(const char *name) {
    char path[1024];
    if (g_shotdir[0]) snprintf(path, sizeof(path), "%s/%s.bin", g_shotdir, name);
    else snprintf(path, sizeof(path), "%s.bin", name);
    FILE *f = fopen(path, "wb");
    if (!f) { fprintf(stderr, "screen: cannot write %s\n", path); return; }
    fwrite(g_scr, 1, sizeof(g_scr), f);
    fclose(f);
}

/* ---- the script ---- */
/* The next input line at g_pos: "shot" lines on the way are taken (the screen as it is now), "battle" / "orders" lines are
   left for screen_script_choices, "end" or the end of the lines ends the run (g_quit). NULL = nothing left. */
static sev_t *script_next(void) {
    for (; g_pos < g_nev; g_pos++) {
        sev_t *e = &g_ev[g_pos];
        if (e->used || e->kind == EV_BATTLE || e->kind == EV_ORDERS) continue;
        if (e->kind == EV_SHOT) { e->used = 1; screen_shot(e->s); continue; }
        if (e->kind == EV_END) { e->used = 1; break; }
        return e;
    }
    g_pos = g_nev;
    g_quit = 1;
    return NULL;
}

static int key_name(const char *s) {
    if (!strcmp(s, "esc")) return 0x1B;
    if (!strcmp(s, "enter")) return 0x0D;
    if (!strcmp(s, "space")) return 0x20;
    if (!strcmp(s, "ins")) return 0x10;           /* 0x03AA: Insert (scan 52h) -> 10h */
    if (!strcmp(s, "tab")) return 0x09;
    if (!strcmp(s, "bs")) return 0x08;
    if (s[0] == '0' && s[1] == 'x' && s[2]) return (int)strtol(s, NULL, 16) & 0xFF;
    return (uint8_t)s[0];
}

void screen_script(const char *path, const char *shotdir) {
    free(g_ev); g_ev = NULL; g_nev = g_pos = 0;
    snprintf(g_shotdir, sizeof(g_shotdir), "%s", shotdir ? shotdir : "");
    g_scripted = 1; g_quit = 0;
    g_sx = SW(0xB6); g_sy = SW(0xB8); g_sb = 0;
    FILE *f = fopen(path, "r");
    if (!f) { fprintf(stderr, "screen: cannot read script %s\n", path); g_quit = 1; return; }
    char line[1100];
    int cap = 0;
    while (fgets(line, sizeof(line), f)) {
        char *p = line, cmd[16] = "", arg[1024] = "";
        line[strcspn(line, "\r\n")] = 0;
        while (*p == ' ' || *p == '\t') p++;
        if (!*p || *p == '#') continue;
        sev_t e; memset(&e, 0, sizeof(e));
        int n = 0;
        if (sscanf(p, "%15s %n", cmd, &n) < 1) continue;
        snprintf(arg, sizeof(arg), "%s", p + n);
        if (!strcmp(cmd, "click") || !strcmp(cmd, "rclick")) {
            e.kind = cmd[0] == 'r' ? EV_RCLICK : EV_CLICK;
            if (sscanf(arg, "%d %d", &e.a, &e.b) != 2) { fprintf(stderr, "script: bad line '%s'\n", p); continue; }
        } else if (!strcmp(cmd, "key")) {
            e.kind = EV_KEY;
            if (!arg[0]) e.a = ' ';                    /* "key " + a space */
            else e.a = key_name(arg);
        } else if (!strcmp(cmd, "shot")) {
            e.kind = EV_SHOT; snprintf(e.s, sizeof(e.s), "%s", arg);
        } else if (!strcmp(cmd, "battle")) {
            e.kind = EV_BATTLE;
            if (sscanf(arg, "%d %d", &e.a, &e.b) != 2) { fprintf(stderr, "script: bad line '%s'\n", p); continue; }
        } else if (!strcmp(cmd, "orders")) {
            e.kind = EV_ORDERS; snprintf(e.s, sizeof(e.s), "%s", arg);
        } else if (!strcmp(cmd, "end")) {
            e.kind = EV_END;
        } else { fprintf(stderr, "script: unknown line '%s'\n", p); continue; }
        if (g_nev == cap) { cap = cap ? 2 * cap : 64; g_ev = realloc(g_ev, (size_t)cap * sizeof(*g_ev)); }
        g_ev[g_nev++] = e;
    }
    fclose(f);
}

/* battle_choices for scripted runs: the first unused "battle F T" line; the "orders" lines right after it -> g_orders */
int screen_script_choices(uint8_t *formation, uint8_t *tactic) {
    g_orders[0] = 0;
    for (int i = 0; i < g_nev; i++) {
        if (g_ev[i].used || g_ev[i].kind != EV_BATTLE) continue;
        g_ev[i].used = 1;
        *formation = (uint8_t)g_ev[i].a; *tactic = (uint8_t)g_ev[i].b;
        for (int j = i + 1; j < g_nev && g_ev[j].kind == EV_ORDERS && !g_ev[j].used; j++) {
            g_ev[j].used = 1;
            size_t l = strlen(g_orders);
            snprintf(g_orders + l, sizeof(g_orders) - l, "%s%s", l ? ";" : "", g_ev[j].s);
        }
        fprintf(stderr, "script battle: formation %d, tactic %d%s%s\n", g_ev[i].a, g_ev[i].b, g_orders[0] ? ", orders " : "", g_orders);
        return 1;
    }
    return 0;
}
const char *screen_script_orders(void) { return g_orders[0] ? g_orders : NULL; }

/* ---- the window ---- */
static int us_shift(int k) {
    static const char from[] = "1234567890-=[]\\;',./`", to[] = "!@#$%^&*()_+{}|:\"<>?~";
    const char *p = strchr(from, k);
    return p ? to[p - from] : k;
}

/* show g_scr and read the window's events: pointer, buttons, keys into the key buffer (as the BIOS would give them
   to INT 16h AH=0: AL = the ASCII code; extended keys reach the platform's key_down[] only as Insert = 10h) */
static void pump(void) {
    if (!g_window) return;
    platform_present(g_scr, g_pal);
    platform_poll_input(&g_in);
    if (g_in.quit_requested) g_quit = 1;
    g_wx = g_in.mouse_x; g_wy = g_in.mouse_y;
    g_wb = ((g_in.mouse_buttons & 1) ? 1 : 0) | ((g_in.mouse_buttons & 4) ? 2 : 0);
    SDL_Keymod mod = SDL_GetModState();
    int shift = (mod & KMOD_SHIFT) != 0, ctrl = (mod & KMOD_CTRL) != 0, caps = (mod & KMOD_CAPS) != 0;
    for (int k = 1; k < 256; k++) {
        if (!g_in.key_down[k]) continue;
        int c = k;
        if (k == '=' && shift && g_in.key_down['+']) continue;   /* the platform also sets '+' for Shift+'=' */
        if (k == 0x7F) continue;                                   /* Delete: AL = 0 */
        if (k >= 'a' && k <= 'z') c = ctrl ? (k & 0x1F) : (shift ^ caps) ? k - 0x20 : k;
        else if (shift && k >= 0x20) c = us_shift(k);
        if (g_kn < KBUF) g_kbuf[g_kn++] = (uint8_t)c;
    }
}

void screen_present(void) { pump(); }

/* ---- the original's input routines ---- */
/* 0x0644 mouse poll: [B2] = buttons, [B6] = x, [B8] = y (INT 33h fn 3). take = the poll of 0x359B, which may take the
   script's next click; 0x35DB's polls only see the button go up again. The driver keeps the pointer in the range set
   by 0x06E4 / 0x0709 ([BC]..[C0], [BE]..[C2]). The port always has a mouse (the [BA] = 0 path is not ported). */
static void poll0644(int take) {
    int x, y, b;
    if (g_scripted) {
        g_sb = 0;
        if (take) {
            sev_t *e = script_next();
            if (e && (e->kind == EV_CLICK || e->kind == EV_RCLICK)) {
                e->used = 1; g_pos++;
                g_sx = e->a; g_sy = e->b; g_sb = e->kind == EV_CLICK ? 1 : 2;
            }
        }
        x = g_sx; y = g_sy; b = g_sb;
    } else if (g_window) {
        pump();
        x = g_wx; y = g_wy; b = g_wb;
    } else {                                   /* headless without a script: nothing will ever come */
        g_quit = 1;
        x = SW(0xB6); y = SW(0xB8); b = 0;
    }
    if (SW(0xBC) <= SW(0xC0)) { if (x < SW(0xBC)) x = SW(0xBC); if (x > SW(0xC0)) x = SW(0xC0); }
    if (SW(0xBE) <= SW(0xC2)) { if (y < SW(0xBE)) y = SW(0xBE); if (y > SW(0xC2)) y = SW(0xC2); }
    if (g_scripted) { g_sx = x; g_sy = y; }
    setW(0xB2, (uint16_t)b); setW(0xB6, (uint16_t)x); setW(0xB8, (uint16_t)y);
}

/* 0x0644 for the loops that poll (nego.c: the delay 0x0143, the bedchamber's wait 0x642A): [B2] */
int mouse_poll(void) {
    poll0644(1);
    return W(0xB2);
}

/* 0x066A(x, y): [B6] = x, [B8] = y, INT 33h fn 4 (set the pointer), then 0x0644 */
void mouse_warp(int x, int y) {
    setW(0xB6, (uint16_t)x); setW(0xB8, (uint16_t)y);
    if (g_scripted) { g_sx = x; g_sy = y; }
    else if (g_window) {
        SDL_Window *w = SDL_GetMouseFocus();       /* only while the pointer is over the game's window */
        if (w) { int ww, wh; SDL_GetWindowSize(w, &ww, &wh); SDL_WarpMouseInWindow(w, x * ww / SCREEN_W, y * wh / SCREEN_H); }
    }
    poll0644(0);
}

/* 0x03AA: the next key's ASCII code, 0 when none is waiting; Insert (scan 52h) and scan 9Bh -> 10h; keys with AL = 0
   (arrows, F-keys, ...) and Ctrl-S (13h) are dropped and the next key is read (03CC je 03AA). */
static int key03aa(void) {
    for (;;) {
        int c;
        if (g_scripted) {
            sev_t *e = script_next();
            if (!e || e->kind != EV_KEY) return 0;
            e->used = 1; g_pos++;
            c = e->a;
        } else {
            if (!g_kn) return 0;
            c = g_kbuf[0];
            memmove(g_kbuf, g_kbuf + 1, (size_t)--g_kn);
        }
        if (c == 0x13) c = 0;
        if (c) return c;
    }
}

/* 0x03AA for callers that poll: the window's events are read first (with the screen shown); never blocks */
int key_poll(void) {
    if (!g_scripted) pump();
    return key03aa();
}

/* 0x3603 with a mouse ([BA] != 0, 361B jmp 36E4): the key from 0x03AA as it is */
static int key3603(void) { return key03aa(); }

/* 0x359B: 1 = left button, 2 = right button ([C07F] = 0), 0 = a key in [C07F]. Returns at once while a button is held. */
int wait_input(void) {
    poll0644(1);
    for (;;) {
        if (W(0xB2)) {                                         /* 0x35BF */
            B(0xC07F) = 0;
            return (W(0xB2) & 1) ? 1 : (W(0xB2) & 2) ? 2 : 0;
        }
        if (g_quit) { B(0xC07F) = 0; return 0; }
        poll0644(1);                                           /* 0x35A5 (+ 0x1BE6 cursor, not ported) */
        int k = (g_scripted && W(0xB2)) ? 0 : key3603();       /* script: a click and a key are never taken in one poll */
        if (k) { B(0xC07F) = (uint8_t)k; return 0; }
        if (g_window && !W(0xB2)) platform_sleep_ms(5);
    }
}

/* 0x35DB: until no button is down (tests [B2] before the first poll) */
void wait_release(void) {
    while (W(0xB2) != 0) {
        if (g_quit) { setW(0xB2, 0); break; }
        poll0644(0);
        if (g_window && W(0xB2)) platform_sleep_ms(5);
    }
}

/* 0x35F9 */
void wait_click(void) {
    wait_release();
    wait_input();
    wait_release();
}

/* ---- boxes ---- */
/* 0x59D4 message_box(str): 8C9F(s, 8C58([3E40]), 1, 159, 99) grown by 7; 691B save (screen -> work buffer); 3134 box,
   s in black (8BE4) from Y0 + 7, [3E40] "Click to continue" in white at Y1 - 15 (= ui_message_draw); 35F9; 6973 restore.
   NULL does nothing; "" still shows a box. */
void message_box(const char *s) {
    if (!s) return;
    ui_box_t r = ui_text_rect(s, ui_text_advance(ui_click_text()), 1, 0x9F, 0x63);
    r.x0 -= 7; r.y0 -= 7; r.x1 += 7; r.y1 += 7;
    ui_copy_rect(g_wrk, g_scr, r);                             /* 691B */
    ui_message_draw(g_scr, s);
    wait_click();                                              /* 35F9 */
    ui_copy_rect(g_scr, g_wrk, r);                             /* 6973 */
    if (g_window) pump();
}

/* ui_menu_load on the live DS (the loader reads a whole OVL0 file: DS = file offset - 0xB400) */
void menu_load_ds(menu_t *m, uint16_t rec) {
    static uint8_t img[0xB400 + 0x10000 + 64];
    memcpy(img + 0xB400, g_ds, 0x10000);
    ui_menu_load(m, img, rec);
}

/* 0x54C8 menu_run(rec, preselect) with a mouse (the preselect is not used there): 35DB; 359B until a left click on an
   item (0x552B) or a hotkey (0x5593; a right click is hotkey 0, Esc = 0xFF); 585B(r, ON); 35DB. -1 = Esc (or the end
   of the input). */
static int menu_run54c8(const menu_t *m) {
    int r;
    wait_release();
    do {
        int b = wait_input();
        if (b == 0 && g_quit) return -1;
        r = b == 1 ? ui_menu_hit(m, SW(0xB6), SW(0xB8)) : ui_menu_hotkey(m, B(0xC07F));
    } while (r == -2);
    ui_menu_highlight(g_scr, m, r, 1);
    wait_release();
    return r;
}

/* 0x5614 menu_draw(rec, mode) (mode < 0: not drawn, the caller drew it), the caller's 0x585B(rec, pre, ON) for pre >= 0
   (0x1A31 / 0x1B63 / 0x7D21 / 0x7D76 show the current setting so), then 0x54C8. Index, or -1 for Esc. */
int menu_choose(menu_t *m, int mode, int pre) {
    if (mode >= 0) ui_menu_draw_mode(g_scr, m, mode, -1);
    if (pre >= 0) ui_menu_highlight(g_scr, m, pre, 1);
    return menu_run54c8(m);
}

/* 0x5C20 yes_no(str): the Yes / No record DS:3E5A (items DS:3E42 "Yes" 'Y' / DS:3E4E "No" 'N', no title, single 0,
   enabled) gets its box and item rects here:
     r = 8C9F(s, 0x4A, 1, 159, 99);  X0 = r.x0 - 7, Y0 = r.y0 - 7, X1 = r.x1 + 7, Y1 = r.y1 + 7   -> 3E5A..3E60
     hw = (X1 - X0) / 2:  Yes x0 = hw - 30, x1 = hw - 2;  No x0 = hw + 6, x1 = hw + 30            (3E46 3E4A 3E52 3E56)
     h = Y1 - Y0:  both y0 = h - 16, y1 = h - 6                                                   (3E48 3E54 3E4C 3E58)
   691B(X0, Y0, X1, Y1); 5614(3E5A, 2); [9B69] = 15, 8BE4(s, (X0 + X1) / 2, Y0 + 7); r = 54C8(3E5A, 1); 6973; 1D8A.
   Returns 1 for Yes (r == 0: cmp r, 1; sbb; neg), 0 for No or Esc. */
int yes_no(const char *s) {
    ui_box_t r = ui_text_rect(s, 0x4A, 1, 0x9F, 0x63);
    setW(0x3E5A, (uint16_t)r.x0); setW(0x3E5C, (uint16_t)r.y0); setW(0x3E5E, (uint16_t)r.x1); setW(0x3E60, (uint16_t)r.y1);
    setW(0x3E5E, (uint16_t)(W(0x3E5E) + 7));
    setW(0x3E5A, (uint16_t)(W(0x3E5A) - 7));
    int X0 = SW(0x3E5A), X1 = SW(0x3E5E);
    int hw = cdiv2((int16_t)(X1 - X0));
    setW(0x3E46, (uint16_t)(hw - 0x1E));
    setW(0x3E4A, (uint16_t)(hw - 2));
    setW(0x3E52, (uint16_t)(hw + 6));
    setW(0x3E56, (uint16_t)(hw + 0x1E));
    setW(0x3E60, (uint16_t)(W(0x3E60) + 7));
    setW(0x3E5C, (uint16_t)(W(0x3E5C) - 7));
    int Y0 = SW(0x3E5C), Y1 = SW(0x3E60);
    int h = (int16_t)(Y1 - Y0);
    setW(0x3E54, (uint16_t)(h - 0x10)); setW(0x3E48, (uint16_t)(h - 0x10));
    setW(0x3E58, (uint16_t)(h - 6));    setW(0x3E4C, (uint16_t)(h - 6));
    ui_box_t box = { X0, Y0, X1, Y1 };
    ui_copy_rect(g_wrk, g_scr, box);                           /* 691B */
    menu_t m;
    menu_load_ds(&m, 0x3E5A);
    ui_menu_draw_mode(g_scr, &m, 2, -1);                       /* 5614(3E5A, 2) */
    ui_text_block(g_scr, cdiv2(X1 + X0), Y0 + 7, s, 15);       /* 8BE4 in white */
    int a = menu_run54c8(&m);                                  /* 54C8(3E5A, 1) */
    ui_copy_rect(g_scr, g_wrk, box);                           /* 6973 */
    if (g_window) pump();
    return a == 0;
}

void screen_close(void) {
    if (g_scripted) script_next();             /* take the "shot" lines left at the end */
    if (g_window) platform_shutdown();
    g_window = 0;
    free(g_ovl0); g_ovl0 = NULL;
    free(g_ev); g_ev = NULL; g_nev = g_pos = 0; g_scripted = 0;
}
