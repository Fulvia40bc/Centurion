/* The province command "Legions": literal translation of EXE 0x4065 and the routines it calls (segment 0823,
 * disasm/EXE_RUNTIME.clean.asm; docs/LEGIONS_NOTES.md). Claude, 2026-11-09.
 *
 * Works on the DS image (state.h) with the original addresses and widths. Legion record rec = DS:1918 + 20*L,
 * province P = DS:5270 + 32*z, fleet F = DS:110E + 12*f, army A = DS:C112 + 14*i.
 *
 * The UI is not here (legions.h): messages, questions, menus and clicks go through legions_message / legions_yesno /
 * legions_menu / legions_click, and every drawing call through legions_ui, at the places the original makes them.
 * The DS writes made by the drawing routines of this group (3CB0, 3DB7, 3EA3, 3F23, 4280, 44E5, 4510: the box
 * bookkeeping 1B52/1B84/1B8C/1B90, the colours 9AFA/9B69, the numeral buffer C072) are done here, as in the original;
 * so are the state words written by 55EA/55FF (menu+0x10) and 5D20 ([3EEE] = 0xFF), and 4733's message buffer AC60.
 * The writes inside the shared UI engine (54C8, 5614, 56B6, 57F7, 59D4, 5C20, 494A ...) belong to the UI.
 *
 * Battles: battle_auto() (battle.c) where the original calls the thunk 0xB06E. When no battle choices are available it
 * returns 0: reported on stderr, g_enemy_unported = 1, and the battle counts as not won (nothing else changes).
 * The negotiation (5F82 from 0x602E: 6570 the screen, the dialogue 604B..64DD, the Cleopatra scenes) is nego.c's
 * nego_run(); this file does 5F82's rebel branch, the cursor push / pop around it and 64E0's 53F8. Unlike this file, nego.c
 * draws and reads its input itself (screen.c / ui.c), and its "Go to War" battle comes back here through battle_b06e.
 */
#include "legions.h"
#include "nego.h"
#include "state.h"
#include <stdio.h>
#include <string.h>

#define REC(L)   ((uint16_t)(0x1918 + 20 * (uint16_t)(L)))
#define PROV(z)  ((uint16_t)((uint16_t)(z) << 5))          /* + 0x5270.. */
#define FLEET(f) ((uint16_t)(0x110E + 12 * (uint16_t)(f)))
#define AC60     0xAC60u
#define C072     0xC072u

static const char *dstr(uint16_t a) { return (const char *)&g_ds[a]; }
static void ds_strcpy(uint16_t dst, const char *s) { size_t n = strlen(s); memmove(&g_ds[dst], s, n + 1); }      /* 9E32 */
static void ds_strcat(uint16_t dst, const char *s) { ds_strcpy((uint16_t)(dst + strlen(dstr(dst))), s); }      /* 9DF2 */

/* ---------------------------------------------------------------- hooks and headless defaults */
static const int *g_script; static int g_script_n, g_script_i;
void legions_script(const int *answers, int n) { g_script = answers; g_script_n = n; g_script_i = 0; }
static int script_next(int *v) { if (!g_script || g_script_i >= g_script_n) return 0; *v = g_script[g_script_i++]; return 1; }

static void def_message(const char *t) { fprintf(stderr, "legions: message: %s\n", t); }
static int def_yesno(const char *t) {
    int v = 0; script_next(&v);
    fprintf(stderr, "legions: yes/no: %s -> %s\n", t, v ? "Yes" : "No");
    return v ? 1 : 0;
}
static int def_menu(uint16_t menu, int pre) {
    int v = 0xFF; script_next(&v);
    fprintf(stderr, "legions: menu %04X (preselect %d) -> %d\n", menu, pre, v);
    return v;
}
static int def_click(void) {
    int v = LEGIONS_ESC; script_next(&v);
    fprintf(stderr, "legions: click -> %d\n", v);
    return v;
}
void (*legions_message)(const char *text) = def_message;
int  (*legions_yesno)(const char *text) = def_yesno;
int  (*legions_menu)(uint16_t menu, int preselect) = def_menu;
int  (*legions_click)(void) = def_click;
void (*legions_ui)(int ev, int a, int b, const void *data);

static void ui(int ev, int a, int b, const void *d) { if (legions_ui) legions_ui(ev, a, b, d); }
static void msgbox(uint16_t text) { legions_message(dstr(text)); }              /* 59D4 */
static uint16_t menu_run(uint16_t menu, uint16_t pre) {                         /* 54C8 */
    return (uint16_t)legions_menu(menu, pre);
}

void game_not_ported(const char *what);   /* endturn.c: "<what>\nis not ported yet." box when a screen is open */
static void unported(const char *what, const char *plain, int a, int b) {
    g_enemy_unported = 1;
    fprintf(stderr, "legions: %s (%d, %d) NOT YET PORTED\n", what, a, b);
    game_not_ported(plain);
}

/* B06E: land battle thunk -> overlay DF24(army, L, Z) */
static uint16_t battle_b06e(int A, int L, int Z) {
    uint16_t won = 0;
    if (!battle_auto(A, L, Z, &won)) { unported("land battle 0xB06E (no battle choices)", "This battle", A, L); return 0; }
    return won;
}

/* 6738: number -> Roman numeral in DS:C072; returns C072 */
static uint16_t numeral_6738(uint16_t n) {
    char buf[32];
    roman_numeral(n, buf);
    ds_strcpy(C072, buf);
    return C072;
}

/* 55EA menu_on(menu): [menu+0x10] = 0xFFFF; 56B6 (marker column, title colour 15) is the UI's */
static void menu_on_55EA(uint16_t menu) {
    setW((uint16_t)(menu + 0x10), 0xFFFF);
    ui(LEV_MENU_ON, menu, 0, NULL);
}
/* 55FF menu_off(menu): [menu+0x10] = 0; 57F7 (marker column erased, title colour 6) is the UI's */
static void menu_off_55FF(uint16_t menu) {
    setW((uint16_t)(menu + 0x10), 0);
    ui(LEV_MENU_OFF, menu, 0, NULL);
}
/* 5D20 music_stop: [3EEE] (current music id) = 0xFF, then the sound driver 074B (UI event) */
static void music_stop_5D20(void) {
    B(0x3EEE) = 0xFF;
    ui(LEV_MUSIC_STOP, 0, 0, NULL);
}

/* 4733 at 47C6..480B: the commander message, after battle.c's assign_commander() made the roll and the record writes
   (rec+0x10 = c, [1A20 + 6c] = 0). battle.c's assign_commander does not build this message in DS nor send it through a hook
   (it only prints its own line on stderr, without the numeral), so the tail of 4733 is done here, in the original's order:
   AC60 = [1A1C + 6c] + 1F89 " is assigned\nto command " + 6738(rec numeral) (C072) + rec name; 59D4(AC60).
   If battle.c's assign_commander ever does this itself, drop this call (it would show the message twice). */
static void commander_msg_4733(uint16_t L) {
    uint16_t rec = (uint16_t)(20 * L);
    if (B(rec + 0x1925) == 0xFF) return;                                          /* 474C: 4733 returns at once */
    uint8_t c = B(rec + 0x1928);
    ds_strcpy(AC60, dstr(W((uint16_t)(0x1A1C + 6 * c))));
    ds_strcat(AC60, dstr(0x1F89));
    ds_strcat(AC60, dstr(numeral_6738(B(rec + 0x1922))));
    ds_strcat(AC60, dstr((uint16_t)(rec + 0x1918)));
    legions_message(dstr(AC60));
}

/* ---------------------------------------------------------------- 44E5 / 4510: numbers on the province screen */
static void show_treasury(uint16_t v) {                                          /* 44E5 */
    setW(0x9AFA, W(0x6346)); setW(0x9B69, 0);
    ui(LEV_TREASURY, v, 0, NULL);                                                 /* 8AF2(v, 211, 167, 55) */
}
static void show_recruits(uint16_t v) {                                          /* 4510 */
    setW(0x9AFA, W(0x6346)); setW(0x9B69, 0);
    ui(LEV_RECRUITS, v, 0, NULL);                                                 /* 8AF2(v, 225, 128, 24) */
}

/* ---------------------------------------------------------------- 3F44 scan_zone(z) */
uint8_t legions_scan_zone(uint8_t z) {
    uint8_t n = 0;
    B(0xA6AF) = 0; B(0xA6AE) = 0;
    uint8_t dl = 0;
    for (uint16_t si = 0x1925; si < 0x1A15; si += 0x14, dl++) {
        if (B(si) == 0xFF) continue;
        B(0xA6AE)++;
        if (B(si - 1) != z) continue;
        B(0xA6A2 + n) = dl;
        n++;
        if (B(si + 1) == 0) B(0xA6AF)++;
    }
    return n;
}

/* ---------------------------------------------------------------- 3DB7 legion_row(L, y, select) */
void legions_row_data(uint8_t L, int y, uint16_t select, legions_row_t *r) {
    uint16_t b = (uint16_t)(20 * L);
    memset(r, 0, sizeof(*r));
    r->L = L; r->y = y;
    roman_numeral(B(b + 0x1922), r->numeral);
    r->num_x = select ? 0x23 : 0x19;                                              /* cmp select,1; sbb; and FFF6; add 23 */
    r->name_x = r->num_x + 2;
    r->name = dstr((uint16_t)(b + 0x1918));
    r->type = dstr(W(0x1C64 + 2 * B(b + 0x1925)));
    r->inf = W(b + 0x191E);
    r->cav = W(b + 0x1920);
    if (select == 0) r->col4 = B(b + 0x1929) == 0xFF ? dstr(W(0x5222 + 2 * B(b + 0x1924))) : dstr(W(0x1100 + 2 * B(b + 0x1929)));
    else             r->col4 = dstr(W(0x7486 + 2 * (B(b + 0x1923) / 25)));
    r->status = dstr(W(0x1B54 + 2 * B(b + 0x1926)));
}
static void legion_row_3DB7(uint8_t L, int y, uint16_t select, legions_row_t *r) {
    setW(0x9B69, 0);
    numeral_6738(B(REC(L) + 0xA));
    legions_row_data(L, y, select, r);
}

/* ---------------------------------------------------------------- 3CB0 legion_box(z, title) */
static void legion_box(uint8_t z, uint16_t title) {
    legions_box_t box;
    memset(&box, 0, sizeof(box));
    ui(LEV_CURSOR_PUSH, 0, 0, NULL);
    uint8_t n = legions_scan_zone(z);
    if (n != 0) {
        if (W(0x1B90) == 0) {
            uint8_t step = (uint8_t)(B(0x9B5C) + 2);
            uint16_t si = 5;
            if (title != 0) si = (uint16_t)(step + 5);
            if (n > 3) n = 3;
            B(0x1B8C) = n;
            uint16_t y1 = (uint16_t)((n + 1) * step + si + 7);
            setW(0x1B84, y1); setW(0x1B52, y1);
            box.full = 1; box.rect[0] = 2; box.rect[1] = 0; box.rect[2] = 0x13D; box.rect[3] = y1;   /* 691B + 5614(1B7E, 2) */
            setW(0x9B69, 0);
            box.header = dstr(0x1EFA); box.header_y = si;                        /* 899C(1EFA, 115, si) */
            si = (uint16_t)(si + step);
            box.n = n;
            for (uint8_t i = 0; i < n; i++) {
                legion_row_3DB7(B(0xA6A2 + i), si, 0xFFFF, &box.rows[i]);
                si = (uint16_t)(si + step);
            }
        }
        if (title != 0) {
            setW(0x9AFA, W(0x6346));                                              /* 7E74(7, 5, 312, 15) */
            setW(0x9B69, 0xF);                                                    /* 8BE4(title, 160, 5) */
            box.title = dstr(title);
        }
        ui(LEV_LEGION_BOX, z, title, &box);
        setW(0x1B90, 0xFFFF);
    }
    ui(LEV_CURSOR_POP, 0, 0, NULL);
}

/* ---------------------------------------------------------------- 3EA3 move_header(L): also called by the map loop (08F5..0919) */
void legions_move_header(uint8_t L) {
    legions_box_t box;
    memset(&box, 0, sizeof(box));
    setW(0x1B52, (uint16_t)((B(0x9B5C) + 6) * 2 + B(0x9B5D)));
    ui(LEV_CURSOR_PUSH, 0, 0, NULL);
    box.full = 1; box.rect[0] = 2; box.rect[1] = 0; box.rect[2] = 0x13D; box.rect[3] = W(0x1B52);   /* 691B + 3134(.., [6346]) */
    setW(0x9B69, 0);
    box.header = dstr(0x1EFA); box.header_y = 5;
    box.n = 1;
    legion_row_3DB7(L, B(0x9B5C) + B(0x9B5D) + 5, 0xFFFF, &box.rows[0]);
    ui(LEV_MOVE_HEADER, L, 0, &box);
    ui(LEV_CURSOR_POP, 0, 0, NULL);
}

/* ---------------------------------------------------------------- 3F23 close_box */
void legions_close_box(void) {
    if (W(0x1B90) != 0) {
        int r[4] = { W(0x1B4C), W(0x1B4E), W(0x1B50), W(0x1B52) };
        ui(LEV_RECT_RESTORE, 0, 0, r);
    }
    setW(0x1B90, 0);
}

/* ---------------------------------------------------------------- 4280 legion_list */
static void list_layout(legions_list_t *o, uint8_t n) {
    memset(o, 0, sizeof(*o));
    uint16_t h = (uint8_t)((uint8_t)(n + 3) * B(0x9B5C) + B(0x9B5D) + 0x0C);      /* byte arithmetic */
    int16_t t = (int16_t)(0xC8 - h);
    int16_t top = (int16_t)(t / 2);                                               /* cwd; sub ax,dx; sar 1 */
    o->rect[0] = 2; o->rect[1] = top; o->rect[2] = 0x13D; o->rect[3] = (int16_t)(h + top);
    snprintf(o->title, sizeof(o->title), "%s%s", dstr(0x1F70), dstr(W(0x3E40)));
    o->title_y = top + 5;
    o->header = dstr(0x1ECC);
    o->header_y = (B(0x9B5C) + 3) * 2 + top;
}
void legions_list_data(legions_list_t *o) {
    uint8_t n = 0;
    for (int L = 0; L < 12; L++) if (B(REC(L) + 0xD) != 0xFF) n++;
    list_layout(o, n);
    int si = B(0x9B5D) + o->rect[1] + 5 + 2 * B(0x9B5C);
    for (uint8_t L = 0; L < 12; L++) {
        if (B(REC(L) + 0xD) == 0xFF) continue;
        si += B(0x9B5C);
        legions_row_data(L, si, 0, &o->rows[o->n++]);
    }
}
void legions_list(void) {
    legions_list_t o;
    ui(LEV_CURSOR_PUSH, 0, 0, NULL);
    legions_scan_zone(B(0xC080));
    if (B(0xA6AE) != 0) {
        list_layout(&o, B(0xA6AE));
        int si = B(0x9B5D) + o.rect[1] + 5;
        setW(0x9B69, 0xF);                                                        /* title 8BE4(buf, 160, top + 5) */
        setW(0x9B69, 0);                                                          /* header 899C(1ECC, 115, ..) */
        si += 2 * B(0x9B5C);
        for (uint8_t L = 0; L < 12; L++) {
            if (B(REC(L) + 0xD) == 0xFF) continue;
            si += B(0x9B5C);
            if (o.n < 12) legion_row_3DB7(L, si, 0, &o.rows[o.n++]);
        }
        ui(LEV_LEGION_LIST, 0, 0, &o);
        ui(LEV_WAIT_CLICK, 0, 0, NULL);                                           /* 35F9 */
        ui(LEV_RECT_RESTORE, 0, 0, o.rect);                                       /* 6973(2, top, 317, bottom) */
    }
    ui(LEV_CURSOR_POP, 0, 0, NULL);
}

/* ---------------------------------------------------------------- 4710 draw_legions_menu */
static void draw_legions_menu(void) {
    ui(LEV_CURSOR_PUSH, 0, 0, NULL);
    ui(LEV_WAIT_RELEASE, 0, 0, NULL);
    ui(LEV_MENU_DRAW, 0x1C0A, 1, NULL);
    legion_box(B(0xC080), 0x1D58);
    ui(LEV_CURSOR_POP, 0, 0, NULL);
}

/* ---------------------------------------------------------------- 3BAF select_legion(z, any) */
uint8_t legions_select(uint8_t z, uint16_t any) {
    uint8_t sel = 0x0C;
    uint16_t si = 0;
    uint8_t n = legions_scan_zone(z);
    if (n == 0) return 0xFF;
    if (n > 3) n = 3;
    B(0x1B8C) = n;
    ui(LEV_CURSOR_PUSH, 0, 0, NULL);
    if (n > 1 || B(REC(B(0xA6A2)) + 0xE) != 0)
        legion_box(z, B(0xA6AF) != 0 ? 0x1F5E : 0);
    if (n > 1 && (B(0xA6AF) != 0 || any != si)) {
        menu_on_55EA(0x1B7E);
        while (sel == 0x0C) {
            si = menu_run(0x1B7E, si);
            if (si == 0xFF) sel = 0xFF;
            else if (B(REC(B((uint16_t)(0xA6A2 + si))) + 0xE) != 0 && any == 0) msgbox(0x1D0E);
            else sel = B((uint16_t)(0xA6A2 + si));
        }
        menu_off_55FF(0x1B7E);
    } else {
        sel = B(0xA6A2);
        if (B(REC(sel) + 0xE) != 0 && any == si) {
            msgbox(n > 1 ? 0x1D32 : 0x1D0E);
            sel = 0xFF;
        }
    }
    ui(LEV_CURSOR_POP, 0, 0, NULL);
    return sel;
}

/* ---------------------------------------------------------------- 4392 raise(z, type) */
uint16_t legions_raise(uint8_t z, uint16_t t) {
    uint16_t r = 0;                                                               /* [bp-2] */
    uint16_t p = PROV(z);
    int16_t dx = SW(p + 0x5274);
    if (dx < 0x64) { msgbox(0x1E98); return 0; }
    if (B((uint16_t)(0x1EC8 + t)) > W(0x634A)) {
        msgbox(0x1D6A);
    } else {
        uint16_t inf = W((uint16_t)(0x1AAC + 2 * t)), cav = W((uint16_t)(0x1AB2 + 2 * t));
        if ((int16_t)(inf + cav) > dx) {
            if ((int16_t)cav > dx) { cav = (uint16_t)dx; inf = 0; }
            else inf = (uint16_t)(dx - cav);
        }
        uint16_t L = 0, s;
        for (s = 0x1925; s < 0x1A15; s += 0x14, L++) if (B(s) == 0xFF) break;
        if (s < 0x1A15) {
            uint16_t b = (uint16_t)(20 * L);
            B(b + 0x1925) = (uint8_t)t;
            setW(b + 0x191E, inf);
            setW(b + 0x1920, cav);
            B(b + 0x1923) = B(p + 0x527A);
            B(b + 0x1924) = z;
            B(b + 0x1926) = 2;
            B(b + 0x1927) = 0;
            B(b + 0x192A) = z;
            B(b + 0x1929) = 0xFF;
            ds_strcpy((uint16_t)(b + 0x1918), dstr(W(0x1C2E + 2 * z)));
            B(p + 0x5288)++;
            if (B(p + 0x5288) > 0x32) B(p + 0x5288) = 1;
            B(b + 0x1922) = B(p + 0x5288);
            setW(p + 0x5274, (uint16_t)(W(p + 0x5274) - (uint16_t)(inf + cav)));
            assign_commander((int)L);                                             /* 4733: dice roll(#free commanders) */
            commander_msg_4733(L);                                                /* 4733's message (47C6..480B) */
        }
        setW(0x634A, (uint16_t)(W(0x634A) - B((uint16_t)(0x1EC8 + t))));
        r = 0xFFFF;
    }
    show_treasury(W(0x634A));
    show_recruits(W(p + 0x5274));
    return r;
}

/* ---------------------------------------------------------------- 453B strengthen(L, z) */
void legions_strengthen(uint8_t L, uint8_t z) {
    uint16_t rec = (uint16_t)(20 * L);
    uint8_t t = B(rec + 0x1925);
    uint16_t maxinf = W(0x1AAC + 2 * t), maxcav = W(0x1AB2 + 2 * t);
    uint16_t maxsum = (uint16_t)(maxcav + maxinf);
    if (maxinf == W(rec + 0x191E) && W(rec + 0x1920) == maxcav) { msgbox(0x1E74); return; }
    uint16_t cur = (uint16_t)(W(rec + 0x191E) + W(rec + 0x1920));
    uint16_t cost = B(0x1EC8 + t);
    uint16_t p = PROV(z);
    uint16_t di;
    if (cost > W(0x634A)) di = (uint16_t)(((uint32_t)W(0x634A) * maxsum) / cost);   /* mul word; A048 */
    else di = W(p + 0x5274);
    uint16_t si = W(p + 0x5274);
    if (si > di) si = di;
    uint16_t cav_add;
    if (W(rec + 0x1920) >= maxcav) cav_add = 0;
    else {
        uint16_t ax = (uint16_t)(maxcav - W(rec + 0x1920));
        cav_add = (uint16_t)(W(p + 0x5274) >> 2);
        if (cav_add > si) cav_add = si;
        if (ax < cav_add) cav_add = ax;
    }
    si = (uint16_t)(si - cav_add);
    uint16_t inf_add = 0;
    if (W(rec + 0x191E) < maxinf) {
        uint16_t ax = (uint16_t)(maxinf - W(rec + 0x191E));
        inf_add = si;
        if (ax < si) inf_add = ax;
    }
    setW(rec + 0x1920, (uint16_t)(W(rec + 0x1920) + cav_add));
    setW(rec + 0x191E, (uint16_t)(W(rec + 0x191E) + inf_add));
    uint16_t added = (uint16_t)(inf_add + cav_add);
    setW(p + 0x5274, (uint16_t)(W(p + 0x5274) - added));
    uint16_t div = (uint16_t)(added + cur);
    uint32_t num = (uint32_t)added * B(p + 0x527A) + (uint32_t)B(rec + 0x1923) * cur;   /* 9D04 + mul word */
    if (div == 0)   /* A048 divides by zero in the original (a legion with no men and nothing added) */
        fprintf(stderr, "legions: strengthen legion %u: courage divides by zero in the original; courage kept\n", L);
    else B(rec + 0x1923) = (uint8_t)(num / div);
    setW(0x634A, (uint16_t)(W(0x634A) - (uint16_t)(((uint32_t)cost * added) / maxsum)));
    if (added != 0) B(rec + 0x1926) = 2;                                          /* si = added at 46A0 */
    show_treasury(W(0x634A));
    show_recruits(W(p + 0x5274));
}

/* ---------------------------------------------------------------- 3F97 upgrade(L) */
void legions_upgrade(uint8_t L) {
    char buf[0x50];
    uint16_t rec = (uint16_t)(20 * L);
    const char *msg = NULL;
    buf[0] = 0;
    snprintf(buf, sizeof(buf), "%s", dstr((uint16_t)(rec + 0x1918)));
    switch (B(rec + 0x1925)) {
    case 0:
        if (B(0x5971) < 2) msg = dstr(0x1D9E);
        else if ((uint16_t)(B(0x1EC9) - B(0x1EC8)) > W(0x634A)) msg = dstr(0x1D6A);
        else {
            setW(0x634A, (uint16_t)(W(0x634A) + (uint16_t)(B(0x1EC8) - B(0x1EC9))));
            B(rec + 0x1925) = 1;
            strncat(buf, dstr(0x1E4C), sizeof(buf) - strlen(buf) - 1);
            msg = buf;
        }
        break;
    case 1:
        if (B(0x5971) < 6) msg = dstr(0x1DD0);
        else if ((uint16_t)(B(0x1ECA) - B(0x1EC9)) > W(0x634A)) msg = dstr(0x1D6A);
        else {
            setW(0x634A, (uint16_t)(W(0x634A) + (uint16_t)(B(0x1EC9) - B(0x1ECA))));
            B(rec + 0x1925) = 2;
            strncat(buf, dstr(0x1E26), sizeof(buf) - strlen(buf) - 1);
            msg = buf;
        }
        break;
    case 2:
        msg = dstr(0x1E02);
        break;
    default:
        break;                                                                    /* 4057: no message */
    }
    if (msg) legions_message(msg);
    show_treasury(W(0x634A));
}

/* ---------------------------------------------------------------- 3A32 fleet_fit(f, L, &cav, &inf) */
uint16_t legions_fleet_fit(uint8_t f, uint16_t L, uint16_t *cav, uint16_t *inf) {
    uint32_t cap = 0;
    uint16_t di = FLEET(f);
    for (int t = 0; t < 3; t++, di += 2) cap += (uint32_t)W(di) * B(0x8EDE + t);
    uint16_t bx = (cap >> 16) != 0 || (uint16_t)cap >= 0x2710 ? 0x2710 : (uint16_t)cap;
    uint16_t rec = (uint16_t)(20 * L);
    *cav = W(rec + 0x1920);
    *inf = W(rec + 0x191E);
    uint16_t dx;
    if ((uint16_t)(*cav << 2) >= bx) {
        dx = (uint16_t)(*cav - (bx >> 2));
        *cav = (uint16_t)(bx >> 2);
        bx = 0;
    } else {
        bx = (uint16_t)(bx - (uint16_t)(*cav << 2));
        dx = 0;
    }
    if (*inf >= bx) {
        dx = (uint16_t)(dx + (*inf - bx));
        *inf = bx;
    }
    return dx;
}

/* ---------------------------------------------------------------- 3AE4 board(z0, L, f) */
uint16_t legions_board(uint8_t z0, uint8_t L, uint8_t f) {
    uint16_t cav, inf, sum = 0;
    uint16_t left = legions_fleet_fit(f, L, &cav, &inf);
    if (left != 0) {
        char num[8];
        snprintf(num, sizeof(num), "%d", (int16_t)left);                          /* itoa(left, AC60, 10) */
        ds_strcpy(AC60, num);
        ds_strcat(AC60, dstr(0x1F28));
        if (!legions_yesno(dstr(AC60))) return 0;
    }
    uint16_t rec = (uint16_t)(20 * L);
    setW(rec + 0x1920, cav);
    setW(rec + 0x191E, inf);
    uint16_t F = FLEET(f);
    for (int t = 0; t < 3; t++) sum = (uint16_t)(sum + W(F + 2 * t));
    if (sum == 0) fprintf(stderr, "legions: board: fleet %u has no ships (the original divides by zero here)\n", f);
    else B(F + 0x0A) = (uint8_t)((uint16_t)((uint16_t)(cav << 2) + inf) / sum);
    uint16_t p = PROV(z0);
    setW(p + 0x5274, (uint16_t)add_cap(SW(p + 0x5274), (int16_t)left, SW(p + 0x5276)));   /* 04FA */
    return 0xFFFF;
}

/* ---------------------------------------------------------------- 4BAC army_msg(name, numeral, z, army) */
static void army_msg(uint16_t name, uint16_t numeral, uint16_t z, uint16_t army) {
    setW(0x3D7C, W(0x5222 + 2 * W((uint16_t)(0xC114 + 14 * army))));
    setW(0x3D80, W((uint16_t)(0x5222 + 2 * z)));
    numeral_6738(numeral);
    setW(0x3D86, name);
    setW(0x3D8A, z >= 0x1B ? W(0x3D8E) : W(0x3D8C));
    ds_strcpy(AC60, dstr(W(0x3D7A)));
    for (uint16_t si = 0x3D7C; si < 0x3D8C; si += 2) ds_strcat(AC60, dstr(W(si)));
    legions_message(dstr(AC60));
}

/* ---------------------------------------------------------------- 4B3F fight_armies(L, z) */
uint16_t legions_fight_armies(uint16_t L, uint16_t z) {
    uint16_t r = 0xFFFF;
    uint16_t di = 0;
    for (uint16_t si = 0xC11A; si < 0xC1B4; si += 0x0E, di++) {
        if (W(si) == 0) continue;
        uint16_t ax = B(si - 7);
        if (ax != z) continue;
        uint16_t rec = (uint16_t)(20 * L);
        army_msg((uint16_t)(rec + 0x1918), B(rec + 0x1922), ax, di);
        ui(LEV_DISK_SWAP, 0, 0, NULL);
        if (battle_b06e(di, L, z)) setW(si, 0);
        else r = 0;
    }
    return r;
}

/* ---------------------------------------------------------------- 53F8 province_won(z): enemy fleets based there are gone */
static void province_won(uint16_t z) {
    for (uint16_t bx = 0x1139; bx < 0x115D; bx += 0x0C)
        if (B(bx) == z) B(bx + 1) = 0xFF;
}

/* ---------------------------------------------------------------- 5F82 enter_foreign(L, z) */
uint16_t legions_enter_foreign(uint8_t L, uint8_t z) {
    ui(LEV_WAIT_RELEASE, 0, 0, NULL);
    uint16_t p = PROV(z);
    if (B(p + 0x5287) != 0) {                                                     /* rebels */
        uint16_t rec = (uint16_t)(20 * L);
        ds_strcpy(AC60, dstr(numeral_6738(B(rec + 0x1922))));
        setW(0x4ECC, (uint16_t)(rec + 0x1918));
        setW(0x4ED0, W(0x5222 + 2 * z));
        for (uint16_t si = 0x4ECC; si < 0x4ED4; si += 2) ds_strcat(AC60, dstr(W(si)));
        legions_message(dstr(AC60));
        ui(LEV_DISK_SWAP, 0, 0, NULL);
        uint16_t r = battle_b06e(0xFF, L, z);
        if (r) {
            B(p + 0x5285) = 1;
            B(p + 0x5289) = 0;
            B(p + 0x5282) = B(0x5FC0);
            province_won(z);
        }
        return r;
    }
    ui(LEV_CURSOR_PUSH, 0, 0, NULL);                                              /* 602E: 1D55(0) */
    uint16_t r = nego_run(L, z, battle_b06e);                                     /* 603C: 6570, the dialogue 604B..64DD (nego.c) */
    if (r) province_won(z);                                                       /* 64E0: 53F8 */
    ui(LEV_CURSOR_POP, 0, 0, NULL);                                               /* 64EC: 1D8A (64EF "tab" freed in nego.c) */
    return r;
}

/* ---------------------------------------------------------------- 382A move_legion(L) */
void legions_move(uint8_t L) {
    uint8_t dest = 0xFF;                                                          /* [bp-4] */
    uint16_t done = 0;                                                            /* si */
    uint16_t rec = (uint16_t)(20 * L);
    uint8_t z0 = B(rec + 0x1924);
    uint8_t ship[8];
    ui(LEV_CURSOR_PUSH, 0, 0, NULL);
    for (int k = 0; k < 8; k++) {
        ship[k] = 0xFF;
        uint8_t n = B((uint16_t)(0x50EC + (z0 << 3) + k));
        if (n < 0x1B) ui(LEV_HIGHLIGHT, n, 0, NULL);
        if (z0 >= 0x1B) continue;
        uint8_t f = 0;
        for (uint16_t si = 0x1114; si < 0x1138; si += 0x0C, f++) {
            if (B(si) == n && B(si + 2) != 0xFF && B(si + 3) == 0xFF) {
                ship[k] = f;
                ui(LEV_HIGHLIGHT, n, 1, NULL);
                break;
            }
        }
    }
    ui(LEV_CURSOR_SHOW, 0, 0, NULL);
    while (done == 0) {
        ui(LEV_WAIT_RELEASE, 0, 0, NULL);
        int c = legions_click();                                                  /* 359B, then 6C67([B6], [B8]) */
        if (c < 0) { if (c == LEGIONS_ESC) break; continue; }
        uint8_t z = (uint8_t)c;
        if (z == z0) done = 0xFFFF;
        for (int k = 0; k < 8; k++) {
            if (B((uint16_t)(0x50EC + (z0 << 3) + k)) != z) continue;
            uint8_t f = ship[k];
            if (f != 0xFF && legions_board(z0, L, f)) {
                B((uint16_t)(0x1117 + 12 * f)) = L;
                B(rec + 0x1929) = f;
                B(rec + 0x1926) = 1;
                B(rec + 0x1924) = z;
                done = 0xFFFF;
            }
            if (z >= 0x1B) continue;
            B(rec + 0x1926) = 1;
            music_stop_5D20();
            if (legions_fight_armies(L, z)) {
                uint16_t res;
                if (B(PROV(z) + 0x5285) == 0) {
                    res = legions_enter_foreign(L, z);
                    if (res) score_add(z > 5 ? 5 : z == 2 ? 0x1E : 0x0F);
                } else res = 0xFFFF;
                if (res) {
                    B(rec + 0x1924) = z;
                    dest = z;
                    if (z0 >= 0x1B) {
                        uint16_t s = (uint16_t)(12 * B(rec + 0x1929));
                        B((uint16_t)(s + 0x1117)) = 0xFF;
                        B(rec + 0x1929) = 0xFF;
                    }
                }
            } else dest = z0;
            done = 0xFFFF;
        }
    }
    ui(LEV_MAP_REDRAW, dest, 0, NULL);
    ui(LEV_CURSOR_POP, 0, 0, NULL);
}

/* ---------------------------------------------------------------- 4065 the command */
int legions_cmd(void) {
    uint16_t done = 0;                                                            /* [bp-4] */
    uint16_t item = 0;                                                            /* di */
    uint16_t type_sel = 0;                                                        /* [bp-8], kept between raises */
    uint8_t L;
    ui(LEV_CURSOR_PUSH, 0, 0, NULL);
    draw_legions_menu();
    while (done == 0) {
        item = menu_run(0x1C0A, item);
        switch (item) {
        case 0: {                                                                 /* 40C7 Raise Legion */
            ui(LEV_WAIT_RELEASE, 0, 0, NULL);
            if (B(0xA6AE) == 0x0C) { msgbox(0x1C9C); break; }
            if (B(0x595E + B(0x5971)) <= B(0xA6AE)) { msgbox(0x1CBA); break; }
            uint16_t save = item;                                                 /* [bp-6] */
            ui(LEV_MENU_DRAW, 0x1C1C, 1, NULL);
            uint16_t stop = 0;                                                    /* di = [bp-2] = 0 */
            while (stop == 0) {
                type_sel = menu_run(0x1C1C, type_sel);
                int do_raise = 0;
                if (type_sel == 0) do_raise = 1;
                else if (type_sel == 1 || type_sel == 2) {
                    if (type_sel == 2 && B(0x5971) < 6) msgbox(0x1DD0);
                    else if (B(0x5971) < 2) msgbox(0x1D9E);
                    else do_raise = 1;
                } else stop = 0xFFFF;                                             /* None, Esc */
                if (do_raise && legions_raise(B(0xC080), type_sel)) stop = 0xFFFF;
                ui(LEV_MENU_MARK, 0x1C1C, type_sel, NULL);
            }
            legions_close_box();
            draw_legions_menu();
            item = save;
            break;
        }
        case 1:                                                                   /* 4168 Strengthen Legion */
            menu_off_55FF(0x1C0A);
            L = legions_select(B(0xC080), 0);
            if (L < 0x0C) { legions_strengthen(L, B(0xC080)); legions_close_box(); }
            legion_box(B(0xC080), 0x1D58);
            menu_on_55EA(0x1C0A);
            break;
        case 2:                                                                   /* 41A2 Move Legion */
            menu_off_55FF(0x1C0A);
            L = legions_select(B(0xC080), 0);
            menu_on_55EA(0x1C0A);
            if (L < 0x0C) {                                                       /* 420D */
                legions_close_box();
                ui(LEV_MAP_REDRAW, B(0xC080), 0, NULL);
                legions_move_header(L);
                ui(LEV_CURSOR_SHOW, 0, 0, NULL);
                legions_move(L);
                ui(LEV_CURSOR_POP, 0, 0, NULL);
                return 0xFFFF;
            }
            legion_box(B(0xC080), 0x1D58);
            break;
        case 3:                                                                   /* 41D0 Upgrade Legion */
            menu_off_55FF(0x1C0A);
            L = legions_select(B(0xC080), 0xFFFF);
            if (L < 0x0C) { legions_upgrade(L); legions_close_box(); }
            legion_box(B(0xC080), 0x1D58);
            menu_on_55EA(0x1C0A);
            break;
        case 4:                                                                   /* 41EF Legion List */
            legions_close_box();
            legions_list();
            legion_box(B(0xC080), 0x1D58);
            break;
        case 5: case 0xFF:                                                        /* 41F7 Continue / Esc */
            legions_close_box();
            done = 0xFFFF;
            break;
        default:
            break;
        }
        ui(LEV_MENU_MARK, 0x1C0A, item, NULL);                                    /* 41FF */
    }
    ui(LEV_CURSOR_POP, 0, 0, NULL);
    return 0;
}

/* ================================================================ self-test */
/* All self-test output goes to stderr, like the hooks' defaults, so the two stay in order. */
static uint8_t g_before[0x10000];
static const struct { uint16_t a, n; const char *what; } g_watch[] = {
    { 0x00C8, 4, "dice" }, { 0x110E, 72, "fleets" }, { 0x1918, 260, "legions" }, { 0x1A1C, 144, "commanders" },
    { 0x1B52, 2, "box y1" }, { 0x1B84, 2, "menu y1" }, { 0x1B8C, 1, "menu n" }, { 0x1B8E, 2, "1B7E on" }, { 0x1B90, 2, "box up" },
    { 0x1C1A, 2, "1C0A on" }, { 0x3EEE, 1, "music" },
    { 0x5270, 864, "provinces" }, { 0x594A, 2, "score" }, { 0x634A, 2, "treasury" }, { 0xA6A2, 14, "scan" },
    { 0xC112, 154, "armies" },
};
static void snap(void) { memcpy(g_before, g_ds, sizeof(g_before)); }
static void print_effects(void) {
    for (size_t w = 0; w < sizeof(g_watch) / sizeof(g_watch[0]); w++) {
        int any = 0;
        for (uint16_t a = g_watch[w].a; a < g_watch[w].a + g_watch[w].n; a++) {
            if (g_before[a] == g_ds[a]) continue;
            if (!any) { fprintf(stderr, "    %-10s", g_watch[w].what); any = 1; }
            fprintf(stderr, " %04X:%02X>%02X", a, g_before[a], g_ds[a]);
        }
        if (any) fprintf(stderr, "\n");
    }
}
static void print_legions(void) {
    for (int L = 0; L < 12; L++) {
        uint16_t r = REC(L);
        if (B(r + 0xD) == 0xFF) continue;
        char num[16]; roman_numeral(B(r + 0xA), num);
        fprintf(stderr, "    L%d %s%s %s inf %u cav %u courage %u zone %u status %u fleet %u cmdr %u home %u\n", L, num, dstr(r),
               dstr(W(0x1C64 + 2 * B(r + 0xD))), W(r + 6), W(r + 8), B(r + 0xB), B(r + 0xC), B(r + 0xE), B(r + 0x11),
               B(r + 0x10), B(r + 0x12));
    }
}
static int st_choices(uint8_t *f, uint8_t *t) { *f = 0; *t = 0; return 1; }
static int load(const char *path) {
    if (state_load_mem(path) != 0) { fprintf(stderr, "cannot load %s\n", path); return -1; }
    return 0;
}
#define RUN(title, ...) do { static const int a_[] = { __VA_ARGS__ }; legions_script(a_, (int)(sizeof(a_) / sizeof(a_[0]))); \
                             fprintf(stderr, "-- %s\n", title); fflush(stderr); snap(); } while (0)

int legions_selftest(const char *ds_path) {
    const char *path = ds_path ? ds_path : "C:\\work\\Centurion\\tools\\dosbox\\results\\endturn\\t10_in.bin";
    int (*saved_choices)(uint8_t *, uint8_t *) = battle_choices;
    if (!battle_choices) battle_choices = st_choices;   /* battles: formation 0, tactic 0 unless --choices was given */
    int r;
    if (load(path)) return 1;
    fprintf(stderr, "DS %s: treasury %u, rank %u, score %u\n", path, W(0x634A), B(0x5971), W(0x594A));
    print_legions();

    /* 1. Italia, rank 0 (1 legion allowed, 1 raised): Raise, then Continue */
    B(0xC080) = 0;
    RUN("1. Italia, rank 0: Raise -> refused; Continue", 0, 5);
    r = legions_cmd(); fprintf(stderr, "    returns %d\n", r); print_effects();

    /* 2. rank set to 6: Raise a Consular Army in Italia, then Continue */
    B(0x5971) = 6;
    RUN("2. Italia, rank set to 6: Raise, Consular Army; Continue", 0, 2, 5);
    r = legions_cmd(); fprintf(stderr, "    returns %d\n", r); print_effects(); print_legions();

    /* 3. the same legion again: Upgrade -> "Consular Armies may not be upgraded"; Legion List; Continue */
    RUN("3. Italia: Upgrade (a Consular Army), Legion List, Continue", 3, 4, 5);
    r = legions_cmd(); fprintf(stderr, "    returns %d\n", r); print_effects();
    { legions_list_t o; legions_list_data(&o);
      fprintf(stderr, "    list: rect %d,%d-%d,%d title \"%s\" at y %d, header at y %d\n", o.rect[0], o.rect[1], o.rect[2], o.rect[3],
             o.title, o.title_y, o.header_y);
      for (int i = 0; i < o.n; i++)
          fprintf(stderr, "      y %3d  %s%s %s %u %u %s %s\n", o.rows[i].y, o.rows[i].numeral, o.rows[i].name, o.rows[i].type,
                 o.rows[i].inf, o.rows[i].cav, o.rows[i].col4, o.rows[i].status); }

    /* 4. Narbonensis: legion 0 at 3000 infantry: Strengthen; Continue */
    if (load(path)) return 1;
    B(0xC080) = 24; setW(REC(0) + 6, 3000);
    RUN("4. Narbonensis, legion 0 set to 3000 infantry: Strengthen; Continue", 1, 5);
    r = legions_cmd(); fprintf(stderr, "    returns %d\n", r); print_effects(); print_legions();

    /* 5. Narbonensis, rank set to 2: Upgrade twice (to Cavalry, then refused at this rank); Continue */
    if (load(path)) return 1;
    B(0xC080) = 24; B(0x5971) = 2;
    RUN("5. Narbonensis, rank set to 2: Upgrade, Upgrade; Continue", 3, 3, 5);
    r = legions_cmd(); fprintf(stderr, "    returns %d\n", r); print_effects();

    /* 6. Move legion 0 from Narbonensis to Hispania (independent, not in revolt): negotiation */
    if (load(path)) return 1;
    B(0xC080) = 24; g_enemy_unported = 0;
    RUN("6. Narbonensis: Move Legion, click Hispania (15)", 2, 15);
    r = legions_cmd(); fprintf(stderr, "    returns %d, unported %d\n", r, g_enemy_unported); print_effects(); print_legions();

    /* 7. Move legion 0 to Alpes after setting Alpes +0x17 (rebels): the rebel battle */
    if (load(path)) return 1;
    B(0xC080) = 24; B(PROV(11) + 0x5287) = 0xFF; g_enemy_unported = 0;
    RUN("7. Narbonensis: Move Legion, click Alpes (11), Alpes set in revolt", 2, 11);
    r = legions_cmd(); fprintf(stderr, "    returns %d, unported %d\n", r, g_enemy_unported); print_effects(); print_legions();

    /* 8. Boarding: Roman fleet 0 set to 50 type-0 ships in zone 33 (next to Narbonensis); No, then Yes; then land in Narbonensis */
    if (load(path)) return 1;
    B(0xC080) = 24; setW(FLEET(0), 50); B(FLEET(0) + 6) = 33; B(FLEET(0) + 8) = 0; B(FLEET(0) + 9) = 0xFF;
    RUN("8a. Narbonensis, fleet 0 = 50 ships in zone 33: Move Legion, click 33, No, click 33, Yes", 2, 33, 0, 33, 1);
    r = legions_cmd(); fprintf(stderr, "    returns %d\n", r); print_effects(); print_legions();
    RUN("8b. legion 0 at sea: move (382A), click Narbonensis (24)", 24);
    legions_move(0); print_effects(); print_legions();

    /* 9. Raise in Italia (rank set to 2) and attack the army in Dalmatia (army 1, zone 13) */
    if (load(path)) return 1;
    B(0xC080) = 0; B(0x5971) = 2; g_enemy_unported = 0;
    RUN("9a. Italia, rank set to 2: Raise Infantry; Continue", 0, 0, 5);
    r = legions_cmd(); fprintf(stderr, "    returns %d\n", r); print_effects(); print_legions();
    B(REC(1) + 0xE) = 0;
    RUN("9b. Italia, legion 1 set Ready: Move Legion, click Dalmatia (13)", 2, 13);
    r = legions_cmd(); fprintf(stderr, "    returns %d, unported %d\n", r, g_enemy_unported); print_effects(); print_legions();

    /* 10. The map loop's path (EXE 08F5..0919): select in Narbonensis, close, move header, move -> click Alpes (rebels) */
    if (load(path)) return 1;
    B(PROV(11) + 0x5287) = 0xFF; g_enemy_unported = 0;
    RUN("10. map loop: 3BAF(24, 0), 3F23, 3EA3, 382A, click Alpes (11), Alpes set in revolt", 11);
    {
        uint8_t L = legions_select(24, 0);
        legions_close_box();
        if (L < 12) { legions_move_header(L); legions_move(L); }
        fprintf(stderr, "    selected %u, unported %d\n", L, g_enemy_unported);
    }
    print_effects(); print_legions();

    battle_choices = saved_choices;
    legions_script(NULL, 0);
    return 0;
}
