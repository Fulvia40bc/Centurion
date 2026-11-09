/* The negotiation with a foreign province: EXE 0x5F82 from 0x602E (the dialogue loop 0x604B..0x64DD and the Cleopatra
 * scenes), 0x6570 (the screen), 0x7B56 (the scout report) and the helpers they call: 0x04B8 text, 0x6501 variant,
 * 0x5AE4 / 0x5AC2 the speech log, 0x7CB2 "Click to continue", 0x32E2 shadow, 0x4852 / 0x48EF the bedchamber animation,
 * 0x654B the flash, 0x0143 / 0x00F8 / 0x01AE the timer, 0x5D29 / 0x5D20 / 0x5E1A music. Segment 0823
 * (disasm/EXE_RUNTIME.clean.asm). Claude, 2026-11-09. docs/NEGO_LOGIC_NOTES.md (the loop, D), NEGO_SCREEN_NOTES.md (the
 * drawing, D), NEGO_DATA_NOTES.md (MNEGO), the texts in docs/nego_texts.md.
 *
 * A direct port: the DS image (state.h) is read and written at the original addresses and widths, in the original's
 * order, the dice (0x076B) are rolled where the original rolls them (70BA roll(100) [+ roll(4)] inside 6570, roll(3) in
 * 6501 for every line the province says, roll(12) in the bedchamber wait); the drawing goes through ui.c / screen.c and
 * province.c's strip routines onto g_scr (VGA A000) and g_wrk (the work buffer) as 0x67E4 / 0x680F select them.
 *
 * Port decisions:
 *  - The sheets "tab" / "love1" / "love2" ([C1EE] / [C10E] / [C1EC]) and the text buffer hold DOS heap addresses in the
 *    original: they are kept in C structs here, those DS words are not written.
 *  - Music (5D29 + 5D73 in map.c, 5D20 / 5E1A here): only the DS part ([3EEE], the slot table DS:A6B0, with 5D73's
 *    slot bookkeeping: with the sound on ([3EEC] != 0) "cn6" takes a free slot, which 5E1A leaves as {0, 0, 0xFF} as the
 *    original does; the handle word holds 1 instead of a DOS segment while it is loaded); no sound (the loading of
 *    Cleopatra's love theme CN6.LZW and the driver are not ported).
 *  - Time: in the window the 72.8 Hz timer comes from platform_ticks_ms. Headless (scripts) no time passes: every delay
 *    0x0143 and every 14-tick step of 0x48EF is over at once and takes no input, so a script never loses a line to a
 *    delay; the bedchamber's wait loop (0x642A) then ends at the script's next click or key after the one flash it always
 *    shows, without a roll(12) (as a player who clicks at once). When the input ends (window closed, script over) that
 *    loop ends too.
 *  - When the input ends while the reply menu waits, the talk ends as Withdraw does, without its line (port only).
 *  - 1990 bugs, fixed by default and kept with --original (g_original_bugs):
 *    1. mood = [634E] / 34 reaches 3 when [634E] >= 102 (an undefended province taken by an enemy army multiplies [634E]
 *       by 1.25 with no cap, enemy.c 0x5175): the A table read then runs into the next table. Fixed: mood at most 2.
 *       With --original the read is done as is; 0x04B8 copies an empty text for a state beyond the file (the original
 *       copies whatever the offsets give: G a crash or garbage) and says so on stderr.
 *    2. The scout report's three word tables (5F52 wealth, 5F5A army, 5F62 courage) have 4 entries and the index is not
 *       clamped: Aegyptus (wealth 200) is "ordinary". Fixed: index at most 3.
 *  - Not ported: the mouse cursor (1D55 / 1D8A / 1CCE), the disk check 0x1E21 (legions_ui LEV_DISK_SWAP, nothing in the
 *    port), the Tandy / EGA paths of 67E4 / 680F / 6837.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nego.h"
#include "state.h"
#include "screen.h"
#include "assets.h"
#include "platform.h"
#include "legions.h"

#define PZ(z)   ((uint16_t)(0x5270 + 32 * (uint16_t)(z)))   /* province record P */
#define REC(L)  ((uint16_t)(0x1918 + 20 * (uint16_t)(L)))   /* legion record */
#define AC60    0xAC60u

static const char *dstr(uint16_t a) { return (const char *)&g_ds[a]; }
static size_t ds_len(uint16_t a) { size_t n = 0; while ((size_t)a + n < 0x10000 && g_ds[a + n]) n++; return n; }
/* 9DF2 strcat(dst, src) / 9E32 strcpy(dst, src) on DS strings (kept inside the segment) */
static void ds_strcat(uint16_t d, uint16_t s) {
    size_t at = (size_t)d + ds_len(d), n = ds_len(s);
    if (at + n + 1 > 0x10000) n = at < 0xFFFF ? 0xFFFF - at : 0;
    memmove(&g_ds[at], &g_ds[s], n);
    if (at < 0x10000) g_ds[at + n] = 0;
}
static void ds_strcpy(uint16_t d, uint16_t s) { g_ds[d] = 0; if (s != d) ds_strcat(d, s); }

/* ---------------------------------------------------------------- drawing targets and primitives */
static uint8_t *g_fb = g_scr;                                   /* [C0D0]: 0x67E4 work buffer / 0x680F screen */
static void target_back_67E4(void) { g_fb = g_wrk; }
static void target_screen_680F(void) { g_fb = g_scr; }
static ui_box_t box_of(int x0, int y0, int x1, int y1) { ui_box_t r = { x0, y0, x1, y1 }; return r; }
static void save_691B(int x0, int y0, int x1, int y1) { ui_copy_rect(g_wrk, g_scr, box_of(x0, y0, x1, y1)); }
static void restore_6973(int x0, int y0, int x1, int y1) { ui_copy_rect(g_scr, g_wrk, box_of(x0, y0, x1, y1)); }
/* 6837 (VGA): [4F96] == 0 -> 6887: the whole work buffer to the screen; [4F96] = 0; the screen is the target */
static void page_6837(void) {
    if (W(0x4F96) == 0) memcpy(g_scr, g_wrk, sizeof(g_scr));
    setW(0x4F96, 0);
    g_fb = g_scr;
}
/* 7E74(x0, y0, x1, y1) in [9AFA], inclusive, corners swapped if reversed, clipped */
static void fill_7E74(int x0, int y0, int x1, int y1) {
    if (x1 < x0) { int t = x0; x0 = x1; x1 = t; }
    if (y1 < y0) { int t = y0; y0 = y1; y1 = t; }
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > SCREEN_W - 1) x1 = SCREEN_W - 1;
    if (y1 > SCREEN_H - 1) y1 = SCREEN_H - 1;
    for (int y = y0; y <= y1 && x1 >= x0; y++) memset(g_fb + y * SCREEN_W + x0, (uint8_t)W(0x9AFA), (size_t)(x1 - x0 + 1));
}
/* 950E(src, dst, &rect, dx, dy) with src = dst = the current target: the inclusive rect to (dx, dy), rows top to bottom */
static void copy_950E(int x0, int y0, int x1, int y1, int dx, int dy) {
    for (int y = y0; y <= y1; y++) {
        int ty = dy + (y - y0);
        if (y < 0 || y >= SCREEN_H || ty < 0 || ty >= SCREEN_H || x1 < x0) continue;
        memmove(g_fb + ty * SCREEN_W + dx, g_fb + y * SCREEN_W + x0, (size_t)(x1 - x0 + 1));
    }
}
/* 8D56's colour: ink nibble [9B69] & 0x0F through the colour table DS:60C0 + 16 * (([9B69] >> 8) & 0x3F) (table 0 is the
   identity; [6348] = 010C is table 1 entry 12 = 0x1A, the speech box fill: 7CB2's erase). ui.c ignores the table byte. */
static uint8_t ink_8D56(uint16_t a) { return B(0x60C0 + 16 * ((a >> 8) & 0x3F) + (a & 0x0F)); }
/* 899C(s, x, y) in [9B69] (transparent; the opaque band, bit 15, is never used here) */
static void text_899C(const char *s, int x, int y) {
    uint16_t a = W(0x9B69);
    if (a & 0x8000) ui_text_attr(g_fb, x, y, s, a);
    else ui_text(g_fb, x, y, s, ink_8D56(a));
}
/* 8BE4(s, cx, y): at cx - (8C58(s) >> 1) (the strings centred here have one line) */
static void text_8BE4(const char *s, int cx, int y) { text_899C(s, cx - (int)((unsigned)ui_text_advance(s) >> 1), y); }
/* 32E2(x0, y0, x1, y1): the drop shadow, black one pixel out, grey (8) two pixels out */
static void shadow_32E2(int x0, int y0, int x1, int y1) {
    setW(0x9AFA, 0);
    fill_7E74(x0 + 1, y1 + 1, x1 + 1, y1 + 1);                   /* 7ECA */
    fill_7E74(x1 + 1, y0 + 1, x1 + 1, y1 + 1);                   /* 925C */
    setW(0x9AFA, 8);
    fill_7E74(x0 + 2, y1 + 2, x1 + 2, y1 + 2);
    fill_7E74(x1 + 2, y0 + 2, x1 + 2, y1 + 2);
}

/* ---------------------------------------------------------------- archive members: texts and sprite sheets */
static uint8_t *member(const char *name, size_t *len) {
    int k = assets_find(name);
    return k < 0 ? NULL : assets_decode(k, len);
}
typedef struct { uint8_t *p; size_t n; } textbuf_t;            /* 11C4's buffer (the caller's 8 bytes in the original) */
static int load_11C4(const char *name, textbuf_t *b) { b->p = member(name, &b->n); return b->p != NULL; }
static void free_198F(textbuf_t *b) { free(b->p); b->p = NULL; b->n = 0; }

/* 163B sheets: xxxHDR.LZW = 14-byte descriptors {w, xhot, yhot, bytes/row, rows, transparent nibble, colour table, offset},
   xxxDAT.LZW = 4-bit pixels, the offset counting from 2 bytes before the data (as province.c's sheet_load) */
typedef struct { int w, xhot, yhot, bpr, rows, trans, lut; uint32_t off; } spr_t;
typedef struct { spr_t *e; int n; uint8_t *dat; size_t ndat; } sheet_t;
static sheet_t g_tab, g_love1, g_love2;                         /* [C1EE] "tab", [C10E] "love1", [C1EC] "love2" */

static void free_19A0(sheet_t *s) { free(s->e); free(s->dat); memset(s, 0, sizeof(*s)); }
static void sheet_163B(sheet_t *s, uint16_t name) {
    char hn[24], dn[24];
    size_t hl = 0;
    free_19A0(s);
    snprintf(hn, sizeof(hn), "%s%s", dstr(name), dstr(0x0284));   /* "HDR.LZW" */
    snprintf(dn, sizeof(dn), "%s%s", dstr(name), dstr(0x028C));   /* "DAT.LZW" */
    uint8_t *h = member(hn, &hl);
    s->dat = member(dn, &s->ndat);
    if (!h || !s->dat) {
        free(h); free(s->dat); s->dat = NULL;
        fprintf(stderr, "nego: sprite sheet %s missing in MNEGO\n", dstr(name));
        return;
    }
    s->n = (int)(hl / 14);
    s->e = calloc((size_t)s->n + 1, sizeof(spr_t));
    for (int i = 0; i < s->n; i++) {
        const uint8_t *p = h + 14 * i;
        s->e[i].w = p[0] | p[1] << 8;
        s->e[i].xhot = (int16_t)(p[2] | p[3] << 8);
        s->e[i].yhot = (int16_t)(p[4] | p[5] << 8);
        s->e[i].bpr = p[6]; s->e[i].rows = p[7]; s->e[i].trans = p[8]; s->e[i].lut = p[9];
        s->e[i].off = (uint32_t)(p[10] | p[11] << 8 | p[12] << 16 | (uint32_t)p[13] << 24);
    }
    free(h);
}
/* 8E62(desc k of sheet s, x, y): at (x - xhot, y - yhot), the transparent nibble skipped, the entry's colour table */
static void sprite_8E62(const sheet_t *s, int k, int x, int y) {
    if (!s->e || k < 0 || k >= s->n || s->e[k].off < 2) return;
    const spr_t *e = &s->e[k];
    size_t base = e->off - 2;
    x -= e->xhot; y -= e->yhot;
    for (int r = 0; r < e->rows; r++) {
        int Y = y + r;
        if (Y < 0 || Y >= SCREEN_H) continue;
        for (int c = 0; c < e->w; c++) {
            int X = x + c;
            size_t i = base + (size_t)r * e->bpr + c / 2;
            if (X < 0 || X >= SCREEN_W || i >= s->ndat) continue;
            uint8_t n = (c & 1) ? (s->dat[i] & 15) : (s->dat[i] >> 4);
            if (n != e->trans) g_fb[Y * SCREEN_W + X] = B(0x60C0 + 16 * e->lut + n);
        }
    }
}

/* 04B8(&buf, i, dst): text i of the loaded TEXTx / SCOUT file (u16 offsets, then NUL-terminated texts) copied with its
   NUL to DS:dst. No bounds check in the original; the port copies "" for a text beyond the file (see the header). */
static void text_4B8(const textbuf_t *b, unsigned i, uint16_t dst) {
    const uint8_t *p = b->p;
    unsigned cnt = (p && b->n >= 2) ? (unsigned)(p[0] | p[1] << 8) / 2u : 0;   /* off[0] = 2 * (texts + 1) */
    if (i + 1 >= cnt) {
        fprintf(stderr, "nego: text %u beyond the file (%u texts): an empty text (the original reads past the table)\n", i, cnt ? cnt - 1 : 0);
        g_ds[dst] = 0;
        return;
    }
    unsigned a = p[2 * i] | p[2 * i + 1] << 8, e = p[2 * i + 2] | p[2 * i + 3] << 8;
    if (e > b->n || a > e) { g_ds[dst] = 0; return; }
    size_t n = e - a;
    if ((size_t)dst + n > 0x10000) n = 0x10000 - dst;
    memmove(&g_ds[dst], p + a, n);                              /* rep movsb: the text's own NUL comes with it */
}
/* 6501(s): roll(3) + 1 = n; the n-th of the '~'-separated variants (the last one if there are fewer); NULs written into
   the copy at s. DICE: one roll(3) for every text the province says. */
static uint16_t variant_6501(uint16_t s) {
    uint16_t n = (uint8_t)(rng_roll(3) + 1);                    /* inc al; sub ah, ah */
    uint16_t last = s;
    for (uint16_t i = 0; i < n; i++) {
        last = s;
        char *t = strchr((char *)&g_ds[s], '~');                /* 9FF8 */
        if (t) { *t = 0; s = (uint16_t)(t - (char *)g_ds + 1); }
    }
    return last;
}

/* ---------------------------------------------------------------- the speech log */
/* strtok (A0A8) on ' ' (DS:3E6C / 3E70 are " ") */
static char *tok(char *s, char **save) {
    if (!s) s = *save;
    if (!s) return NULL;
    while (*s == ' ') s++;
    if (!*s) { *save = NULL; return NULL; }
    char *e = s;
    while (*e && *e != ' ') e++;
    if (*e) { *e = 0; *save = e + 1; } else *save = NULL;
    return s;
}
/* 5AE4 say(text, colour, prefix): word-wrapped at 186 px into the log at x 120, lines di = [9B5C] + [9B5D] - 1 = 10 px apart
   from [C1D8]; past y 190 the rect DS:3E74 (120, 86 + di)-(305, 190) moves up by di and the last line is cleared in [6346].
   prefix != 0: the text in that character ('"': the province's and the player's lines). */
static void say_5AE4(uint16_t text, uint16_t colour, uint8_t prefix) {
    char buf[0x146 + 2], line[256], *save = NULL;              /* line: 0x42 bytes in the original (no text overflows it) */
    int done = 0;                                              /* [bp-6] */
    uint16_t w = 0;                                            /* [bp-4] */
    uint16_t di = (uint16_t)((W(0x9B5C) & 0xFF) + B(0x9B5D) - 1);
    memset(buf, 0, sizeof(buf));
    setW(0x9B69, colour);
    setW(0x3E76, (uint16_t)(di + 0x56));
    if (prefix) {
        buf[0] = (char)prefix;
        B(0x3E72) = prefix;
        strncpy(buf + 1, dstr(text), 0xFE);                     /* 9EEE */
        strcat(buf, dstr(0x3E72));                              /* 9DF2 */
    } else strncpy(buf, dstr(text), 0x100);
    char *s = tok(buf, &save);                                 /* si */
    line[0] = 0;
    while (!done) {
        if ((uint16_t)(ui_text_advance(s) + w) > 0xBA || s == NULL) {      /* 0x5B71: 8C58 (NULL -> 0) */
            if ((uint16_t)(W(0xC1D8) + di) > 0xBE) {
                copy_950E(SW(0x3E74), SW(0x3E76), SW(0x3E78), SW(0x3E7A), 0x78, 0x56);
                setW(0xC1D8, (uint16_t)(W(0xC1D8) - di));
                setW(0x9AFA, W(0x6346));
                fill_7E74(0x78, SW(0xC1D8), 0x131, SW(0xC1D8) + di);
            }
            text_899C(line, 0x78, SW(0xC1D8));
            setW(0xC1D8, (uint16_t)(W(0xC1D8) + di));
            line[0] = 0;
        }
        if (s == NULL) { done = -1; continue; }
        strncat(line, s, sizeof(line) - strlen(line) - 2);
        strcat(line, " ");                                     /* DS:3E6E */
        w = (uint16_t)ui_text_advance(line);
        s = tok(NULL, &save);
    }
}
/* 5AC2(n): n blank lines (each say("" DS:3E02, 0, 0)) */
static void blank_5AC2(uint16_t n) { for (uint16_t i = 0; i < n; i++) say_5AE4(0x3E02, 0, 0); }

/* ---------------------------------------------------------------- time and input */
static uint32_t g_t0;                                           /* timer slot 0 ([46]/[48]) */
static void timer_F8(void) { g_t0 = platform_ticks_ms(); }
/* 01AE: ticks of the 72.83 Hz timer since 00F8 (headless: no time passes, every wait is over) */
static uint16_t ticks_1AE(void) {
    if (screen_headless()) return 0xFFFF;
    uint32_t t = (uint32_t)(((uint64_t)(platform_ticks_ms() - g_t0) * 7283u) / 100000u);
    return t > 0xFFFF ? 0xFFFF : (uint16_t)t;
}
/* 0143(n): restart slot 0, wait n ticks or until a button is down (0x0644) or a key comes (0x3603 = 0x03AA, consumed) */
static void delay_143(uint16_t n) {
    timer_F8();
    if (screen_headless()) return;
    while (ticks_1AE() < n) {
        if (mouse_poll()) break;
        if (key_poll()) break;
        if (screen_quit()) break;
        platform_sleep_ms(2);
    }
}
/* 7CB2: "Click to continue" ([3E40]) centred at (212, 183) in colour 4, 35F9, then drawn again in [6348] (= the box fill) */
static void click_7CB2(void) {
    setW(0x9B69, 4);
    text_8BE4(dstr(W(0x3E40)), 0xD4, 0xB7);
    wait_click();
    setW(0x9B69, W(0x6348));
    text_8BE4(dstr(W(0x3E40)), 0xD4, 0xB7);
}
static void disk_1E21(void) { if (legions_ui) legions_ui(LEV_DISK_SWAP, 0, 0, NULL); }   /* floppy check: nothing in the port */

/* 5D29 (map.c music_play_5D29, with 5D73's slot bookkeeping) / 5D20 / 5E1A: the DS part only, the slot table DS:A6B0
   {handle, loaded, id} and [3EEE]; no file is loaded and no driver is called. 5E1A(n): the first slot with loaded != 0
   and id n (0x5E29) is freed (868C, nothing here) and set to {0, 0, 0xFF}, and [3EEE] = 0xFF (0x5E6B..0x5E7C). */
static void music_stop_5D20(void) { B(0x3EEE) = 0xFF; }
static void music_unload_5E1A(uint8_t n) {
    for (int i = 0; i < 4; i++) {
        if (B(0xA6B2 + 4 * i) == 0 || B(0xA6B3 + 4 * i) != n) continue;
        setW(0xA6B0 + 4 * i, 0); B(0xA6B2 + 4 * i) = 0; B(0xA6B3 + 4 * i) = 0xFF; B(0x3EEE) = 0xFF;   /* after 868C free */
        break;
    }
}

/* ---------------------------------------------------------------- 7B56 the scout report */
static void scout_7B56(uint8_t z) {
    textbuf_t sb = { 0 };
    setW(0xC1D8, 0x56);
    if (!load_11C4(dstr(0x5EA1), &sb)) return;                 /* "scout.lzw" */
    text_4B8(&sb, z, AC60);
    say_5AE4(AC60, 0, 0);
    blank_5AC2(1);
    uint16_t P = PZ(z);
    ds_strcpy(AC60, 0x5EAB);                                   /* "The wealth of " */
    ds_strcat(AC60, W(0x5222 + 2 * z));
    ds_strcat(AC60, 0x5EBA);                                   /* " is " */
    uint16_t i = (uint8_t)(B(P + 0x14) / 40);                  /* byte div */
    if (!g_original_bugs && i > 3) i = 3;                      /* 1990 bug: Aegyptus (200) read 5F5A[1] "ordinary" */
    ds_strcat(AC60, W(0x5F52 + 2 * i));
    ds_strcat(AC60, 0x5EBF);                                   /* ", the army is " */
    i = (uint16_t)((uint16_t)(W(P + 2) * 3 + W(P + 0)) / 0x9C4);   /* 16-bit sum, unsigned div 2500 */
    if (!g_original_bugs && i > 3) i = 3;
    ds_strcat(AC60, W(0x5F5A + 2 * i));
    ds_strcat(AC60, 0x5ECE);                                   /* " and the soldiers are known for their " */
    i = (uint8_t)(B(P + 0x0A) / 25);
    if (!g_original_bugs && i > 3) i = 3;
    ds_strcat(AC60, W(0x5F62 + 2 * i));
    say_5AE4(AC60, 0, 0);
    click_7CB2();
    blank_5AC2(2);
    ds_strcpy(AC60, W(0x5F1C + 2 * z));                        /* the leader */
    ds_strcat(AC60, 0x5EF5);                                   /* ", the leader of " */
    ds_strcat(AC60, W(0x5222 + 2 * z));
    ds_strcat(AC60, 0x5F06);                                   /* ", offers a greeting:" */
    say_5AE4(AC60, 0, 0);
    free_198F(&sb);
}

/* ---------------------------------------------------------------- 6570 nego_screen(z, &buf) */
static int nego_screen_6570(uint8_t z, textbuf_t *buf) {
    if (assets_load("MNEGO.DIR", "MNEGO.DAT") != 0) {          /* 0F7B(7, &arch) */
        fprintf(stderr, "nego: archive 7 (MNEGO) cannot be opened\n");
        return 0;
    }
    target_back_67E4();                                        /* 0x6589 */
    setW(0x9AFA, W(0x6344));
    fill_7E74(0, 0x41, 0x13F, 0xC7);
    province_strips70ba(g_fb, z);                              /* 70BA: DICE roll(100), roll(4) if a cloud is drawn */
    copy_950E(0x9F, 0, 0xFE, 0x41, 9, 0x50);                   /* the landscape behind the portrait */
    B(0x4EF4) = (uint8_t)(z + 0x40);
    province_cbm(g_fb, dstr(0x4EEE), 8, 0x4F);                 /* "leader?.cbm": 1273, 8E62, 86A0 */
    B(0x4EE8) = (uint8_t)(z + 0x40);                           /* 0x6608 */
    load_11C4(dstr(0x4EE4), buf);                              /* "text?.lzw" (the result is not checked) */
    if (z == 2) { sheet_163B(&g_love1, 0x4ED4); sheet_163B(&g_love2, 0x4EDA); }   /* "love1", "love2" */
    sheet_163B(&g_tab, 0x4EE0);                                /* 0x6646 "tab" */
    target_screen_680F();
    restore_6973(0, 0, 0x13F, 0x40);
    setW(0x9AFA, W(0x6344));
    fill_7E74(0, 0x41, 0x13F, 0xC7);
    province_lines71bb(g_fb);
    setW(0x9B69, 0);
    text_8BE4(dstr(W(0x5222 + 2 * z)), 0xA0, 0x41);            /* the zone name */
    setW(0x9AFA, W(0x6346));
    fill_7E74(0x75, 0x52, 0x133, 0xC0);                        /* the speech box */
    for (int k = 0; k < 4; k++) sprite_8E62(&g_tab, k, 0x71, 0x4F);   /* 0x66BB: its frame, tab 0..3 */
    scout_7B56(z);
    restore_6973(8, 0x4F, 0x67, 0x90);                         /* the portrait, only now */
    shadow_32E2(8, 0x4F, 0x67, 0x90);
    mouse_warp(0x58, 0xAB);                                    /* 066A */
    setW(0xBC, 8); setW(0xC0, 0x67);                           /* 06E4: x range = the reply box */
    setW(0xBE, 0x97); setW(0xC2, 0xC5);                        /* 0709: y range */
    return 1;                                                  /* 11A9 close (the port: the next assets_load frees it); -1 */
}

/* ---------------------------------------------------------------- Cleopatra: 4852 / 48EF / 654B */
static void wait_ticks(uint16_t n) {                            /* while (01AE() < n): not cut short */
    while (ticks_1AE() < n) { screen_present(); platform_sleep_ms(2); }
}
static void step_48EF(int i) {
    if (B(0x3C26 + i)) {
        target_back_67E4();
        sprite_8E62(&g_love1, 1, 0xE0, 0);
        sprite_8E62(&g_love1, 2, B(0x3C26 + i), 0);
        target_screen_680F();
        restore_6973(0xE0, 0, 0x13F, 0x3F);
    }
    wait_ticks(0x0E);
}
static void love_anim_4852(void) {
    if (!g_love1.e) return;                                    /* [C10E] == 0 */
    target_back_67E4();
    sprite_8E62(&g_love1, 0, 0, 0);
    sprite_8E62(&g_love1, 1, 0xE0, 0);
    sprite_8E62(&g_love1, 2, B(0x3C26), 0);
    target_screen_680F();
    restore_6973(0, 0, 0x13F, 0x3F);
    delay_143(0x48);
    int di = 0;
    for (int si = 0; si < 0xB6; si += 0x0E, di++) {           /* love1 3..15 straight onto the screen, not erased */
        timer_F8();
        sprite_8E62(&g_love1, 3 + si / 0x0E, 0x55, 3);
        step_48EF(di);
    }
    for (int si = 0x0D; si < 0x1C; si++) { timer_F8(); step_48EF(si); }
}
static void flash_654B(void) {
    sprite_8E62(&g_love2, 3, 0, 0);                            /* the lightning over the barge */
    delay_143(9);
    restore_6973(0, 0, 0x13F, 0x3F);
}
/* 0x6458: the end of every Cleopatra branch (the caller sets si and result = 0xFFFF) */
static void cleo_end_6458(textbuf_t *buf) {
    free_198F(buf);
    free_19A0(&g_love1);
    free_19A0(&g_love2);
    music_stop_5D20();
    music_unload_5E1A(8);
}

/* ---------------------------------------------------------------- 5F82 from 0x603C */
uint16_t nego_run(uint8_t L, uint8_t z, uint16_t (*b06e)(int army, int L, int z)) {
    textbuf_t buf = { 0 };                                     /* [bp-0x1E] */
    uint16_t P = PZ(z);                                        /* [bp-0x20] = z * 32 */
    uint16_t si, result, sel = 0, saved_si, A, Bt;
    uint8_t state, weak, c, mood, code, x, code_[3];
    if (!nego_screen_6570(z, &buf)) { target_screen_680F(); return 0; }
    /* 0x604B */
    si = 0; result = 0; state = 0; weak = 0;
    mood = (uint8_t)(B(0x634E) / 0x22);                         /* div cl (8 bit) */
    A = W(0x455A + 2 * z);
    Bt = W(0x42EC + 2 * z);
    c = B(0x4322 + z);
    if (SB(P + 0x0E) > 0x4B && mood != 0) mood--;              /* the province's unrest, signed */
    if (!g_original_bugs && mood > 2) {                         /* 1990 bug: the A table has 3 moods */
        fprintf(stderr, "nego: [634E] = %u gives mood %u (past the A table): mood 2 (--original reads past it)\n", B(0x634E), mood);
        mood = 2;
    }
    if ((uint16_t)(W(REC(L) + 6) + W(REC(L) + 8)) < (uint16_t)(W(P + 0) + W(P + 2))) weak = c;   /* legion < province troops */
    if (!(B(0x4092 + z) > B(0x5971)) && !(B(0x40AE + z) > B(0x634E))) state = 1;   /* rank and [634E] high enough */
    while (si == 0) {                                          /* 0x64D9 */
        /* 0x60D8 */
        saved_si = si;
        text_4B8(&buf, state, AC60);
        say_5AE4(variant_6501(AC60), 0x0F, '"');               /* the province speaks */
        B(0x408E) = 3;
        int di = 0;
        for (uint16_t p = 0x405C; p < 0x4080; p += 0x0C, di++) {   /* the reply menu DS:4080, items DS:405C */
            code = B((uint16_t)(Bt + state * 3 + di));
            code_[di] = code;
            if (code == 0x3D) B(0x408E) = 2;
            uint8_t k = B(0x40E6 + code);
            setW(p, W(0x4028 + 2 * k));                        /* the reply's name */
            B(p + 2) = B(0x404A + k);                          /* its hotkey */
        }
        /* 0x6159: the menu, composed in the work buffer */
        target_back_67E4();
        setW(0x9AFA, W(0x6346));
        fill_7E74(0x0A, 0x97, 0x67, 0xC5);
        for (int k = 0; k < 4; k++) sprite_8E62(&g_tab, 4 + k, SW(0x4080), SW(0x4082));   /* tab 4..7, its frame */
        menu_t m;
        menu_load_ds(&m, 0x4080);
        ui_menu_draw_mode(g_fb, &m, 0, -1);                    /* 5614(4080, 0) */
        target_screen_680F();
        restore_6973(8, 0x97, 0x6E, 0xC6);
        do {                                                   /* 0x61B3: Esc is ignored */
            sel = 0;
            int r = menu_choose(&m, -1, -1);                   /* 54C8(4080, 0) */
            sel = r < 0 ? 0xFF : (uint16_t)r;
        } while (sel == 0xFF && !screen_quit());
        if (sel == 0xFF) {                                     /* port: the input ended; leave as Withdraw, without its line */
            free_198F(&buf);
            if (z == 2) { free_19A0(&g_love1); free_19A0(&g_love2); }
            si = 0xFFFF;
            continue;
        }
        /* 0x61CA */
        si = sel;
        code = code_[sel];
        x = B(0x40E6 + code);                                  /* the reply */
        say_5AE4(W(0x4C92 + 2 * code), 9, '"');                /* the player's line */
        if (x <= 1) {                                          /* 0x647E: Withdraw (0) or Go to War (1) */
            free_198F(&buf);
            if (z == 2) { free_19A0(&g_love1); free_19A0(&g_love2); }
            si = 0xFFFF;
            if (x == 0) { delay_143(0x48); continue; }
            disk_1E21();                                       /* 0x64AB */
            result = b06e(0xFF, L, z);                         /* B06E(z, L, 0xFF): the province's own troops */
            if (result) {
                B(P + 0x15) = 1;                               /* Occupied */
                B(P + 0x19) = 0;
                B(P + 0x12) = B(0x5FC0);                       /* the tribute policy */
            }
            continue;
        }
        /* 0x61F4 */
        delay_143(0x48);
        if (x >= 0x0E)                                         /* Low / Medium / High */
            state = B(0x40CA + z) > (uint8_t)(x - 0x0E) ? 2 : 3;
        else
            state = B((uint16_t)(A + (uint16_t)(c * mood) * 2 + (uint8_t)(B(0x4124 + code) + weak)));
        if (state == 2) {
            /* 0x623C: the alliance; sel is the Low / Medium / High item, so this is the tribute level 1..3 */
            B(P + 0x12) = (uint8_t)(sel + 1);
            B(P + 0x13) = (uint8_t)(sel + 1);
            B(P + 0x15) = 2;                                   /* Ally */
            setW(0xC106, (uint16_t)(W(0xC106) + 1));           /* statistics: alliances */
            B(P + 0x19) = 0;
            setW(P + 4, (uint16_t)(W(P + 4) + W(P + 2)));
            setW(P + 4, (uint16_t)(W(P + 4) + W(P + 0)));
            if (W(P + 6) < W(P + 4)) setW(P + 4, W(P + 6));    /* the troops join the manpower pool, capped */
            text_4B8(&buf, state, AC60);
            say_5AE4(variant_6501(AC60), 0x0F, '"');
            if (z != 2) {
                blank_5AC2(1);
                click_7CB2();
                free_198F(&buf);
                si = 0xFFFF;
                result = 0xFFFF;
                continue;                                      /* 0x62BF jmp 6348: z != 2 -> 64D9, the end */
            }
            /* 0x62C2: Aegyptus. The feast, the bedchamber, the second portrait */
            blank_5AC2(1);
            click_7CB2();
            music_play_5D29(8);                                /* "cn6" (MNEGO CN6.LZW), her love theme: no sound in the port */
            sprite_8E62(&g_love2, 2, 0, 0);                    /* her barge on the Nile */
            setW(0x9AFA, W(0x6344));
            fill_7E74(8, 0x97, 0x6B, 0xC6);                    /* the reply menu erased */
            say_5AE4(0x4DD4, 0, 0);                            /* "Cleopatra entertains the Romans ... five days." */
            blank_5AC2(1);
            say_5AE4(0x4E28, 0, 0);                            /* "Infatuated with the Aegyptian queen, ... sharp-tongued as ever." */
            blank_5AC2(1);
            click_7CB2();
            sprite_8E62(&g_love2, 1, 0, 0);                    /* Cleopatra on her couch */
            sprite_8E62(&g_love2, 0, 8, 0x4F);                 /* her second portrait over the first */
            state = 0x0F;
        }
        si = saved_si;                                         /* 0x6345 */
        if (z != 2) continue;                                  /* 0x6348 */
        switch (state) {
        case 0x11: case 0x12: case 0x13:                       /* 0x636C: she calls the guards */
            text_4B8(&buf, state, AC60);
            say_5AE4(variant_6501(AC60), 0x0F, '"');
            blank_5AC2(1);
            delay_143(0x90);
            say_5AE4(0x4D0C, 0, 0);                            /* "Cleopatra calls the guards, ... the alliance remains intact." */
            click_7CB2();
            cleo_end_6458(&buf);
            si = 0xFFFF; result = 0xFFFF;
            break;
        case 0x15:                                             /* 0x63B0: "Then I offer you this gift..." */
            blank_5AC2(2);
            setW(0x9AFA, W(0x6344));
            fill_7E74(8, 0x97, 0x6B, 0xC6);
            love_anim_4852();
            delay_143(0x90);
            setW(0x9AFA, W(0x6344));
            fill_7E74(8, 0x4F, 0x69, 0x92);                    /* the portrait and its shadow erased */
            sprite_8E62(&g_love2, 2, 0, 0);                    /* the barge again */
            setW(0x9B69, 4);
            text_8BE4(dstr(W(0x3E40)), 0xD4, 0xB8);            /* "Click to continue" at y 184, never erased */
            save_691B(0, 0, 0x13F, 0xC7);
            mouse_poll();                                      /* 0x6425 */
            flash_654B();                                      /* 0x6428 jmp 644B: one flash at once */
            while (W(0xB2) == 0) {                             /* 0x644E */
                if (key_poll() || screen_quit()) break;        /* 0x642A: 03AA */
                mouse_poll();
                if (ticks_1AE() > 0x48) {
                    timer_F8();
                    if (rng_roll(12) < 2) flash_654B();        /* DICE: about once a second, 2 in 12 */
                }
                if (!screen_headless() && W(0xB2) == 0) platform_sleep_ms(2);
            }
            page_6837();                                       /* 0x6455 */
            cleo_end_6458(&buf);
            si = 0xFFFF; result = 0xFFFF;
            break;
        case 0x16:                                             /* "My queen, I must go." */
            cleo_end_6458(&buf);
            si = 0xFFFF; result = 0xFFFF;
            break;
        default:                                               /* 0x0F, 0x10, 0x14 and the states below 0x11: talk on */
            break;
        }
    }
    /* 0x64E0: 53F8 and 1D8A are the caller's; 0x64EF: */
    free_19A0(&g_tab);
    return result;
}
