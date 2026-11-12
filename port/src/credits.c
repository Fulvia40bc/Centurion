/* The credits, OVL0 section 1 (file 0x0000-0x09A0, loaded at 0823:B0E0), Claude 2026-11-12.
 * Translated literally from disasm/OVL_CREDITS.asm (section 1 B8B7..BA74, addresses in the comments).
 *
 *   B9A1  the end credits (EXE thunk B0CE, called once, from the victory sequence: section 4 DF70, race.c)
 *   B9D3  one credit: the title (8BE4 at (159, 47) in colour 1) and the names (in colour 0, in the text rect 8C9F of the
 *         names centred on (159, 99), at least as wide as the title), then a wait of n ticks or a key, then the rect from
 *         y 47 down is filled with [6346] (the frame's fill); -1 when a key ended the wait
 *   B8B7  the wait's poll: in VGA ([9B26] == 2) B8FE, then 03AA: -1 on a key
 *   B8FE  every 4 ticks the five colours DS:6ABA (6-bit r, g, b words) go to the DAC entries DS:6AD8 (A2..A6) one place on
 *   B970  the intro's credits (the first 11 pairs, 288 ticks each; not called by the port: the intro is section 1 B694)
 *
 * The frame is 30E6 at (84, 40) on the screen (the menu frame: fill [6346] and the four MMENU corner sprites), drawn once;
 * the 18 pairs DS:6A6E..6AB2 (title, names) 216 ticks each (3.0 s), a key ends the list; then always the last pair DS:6AB6
 * = ("", "\"Beware the ides of March\"") for 360 ticks (4.9 s) or a key. The game then ends (map.c, 0x09AE exit_game).
 *
 * 1990 BUG fixed by default (--original keeps it): B8FE's colour cycle takes the place (counter + k) mod 4 (b93c mov cx, 4;
 * idiv) for the five colours k = 0..4 while its counter DS:6AE6 runs 0..4 (b91f cmp 4, jbe) and B8CF sets all five DAC
 * entries A2..A6: so DAC A6 is never cycled, colour 0 is overwritten by colour 4 in the same entry, and the counter's step
 * 4 repeats step 0 (D). Fix: mod 5 (G that 5 was meant; every other count in the routine is 5). No dice.
 *
 * PORT: one tick (nv_tick) per pass of the wait, as in the naval and race loops; in the window the events are read each
 * pass (the original reads only the keyboard here) and the screen is shown when it changed (the text, the colours).
 * When the input ends (window closed, script run out) the wait ends as by a key. 84BB (the retrace wait) is nothing.
 *
 * Easter egg kept: the last "credit" has no title: "Beware the ides of March" (DS:69FC), after Bill Fiesterman's
 * "Artist Photo". The credits name the tools: Soft-ICE & MagicCV, Plink-86plus, MSC 5.1 / 6.0 & MASM 5.1, Deluxe Paint II
 * (IBM) and II & III (Amiga).
 *
 * NOT VERIFIED against DOSBox: no capture of the credits exists; D (decoded) and checked by headless runs.
 */
#include <stdio.h>
#include <string.h>
#include "state.h"
#include "screen.h"
#include "naval_int.h"

void province_frame30e6(int x, int y);    /* province.c: 0x30E6 on the screen */

static int g_shown;                       /* port: the window shows the current screen */

static void show(void) { if (!screen_headless()) screen_present(); g_shown = 1; }

/* B8FE: every 4 ticks (DS:6AE2/6AE4 the last time) the counter DS:6AE6 0..4 steps and colour k goes to DAC entry
   W(6AD8 + 2 * ((counter + k) mod 4)) (bug: mod 5 meant) */
static void cycle_B8FE(void) {
    uint32_t last = (uint32_t)W(0x6AE2) | (uint32_t)W(0x6AE4) << 16;
    uint32_t now = (uint32_t)W(0x42) | (uint32_t)W(0x44) << 16;
    if (!(last + 4 < now)) return;                                   /* b90d..b919: ja / jae b96d */
    if (++B(0x6AE6) > 4) B(0x6AE6) = 0;                              /* b91b */
    /* b92b: 84BB, the retrace wait (nothing in the port) */
    int m = g_original_bugs ? 4 : 5;                                 /* b93c: mov cx, 4 (bug) */
    for (int di = 0; di < 5; di++) {                                 /* b930: si = 6ABA .. 6AD8 step 6 */
        uint16_t si = (uint16_t)(0x6ABA + 6 * di);
        screen_dac(W((uint16_t)(0x6AD8 + 2 * ((B(0x6AE6) + di) % m))), W(si), W(si + 2), W(si + 4));   /* 01B6 */
    }
    setW(0x6AE2, W(0x42)); setW(0x6AE4, W(0x44));                    /* b95f */
    g_shown = 0;
}

/* B8B7: -1 when a key was typed (port: or the input ended) */
static int poll_B8B7(void) {
    if (W(0x9B26) == 2) cycle_B8FE();                                /* b8b7 */
    if (!screen_scripted()) mouse_poll_input();                      /* port: the window's events */
    if (!g_shown) show();
    if (key_read() & 0xFF) return -1;                                /* b8c1: 03AA */
    if (screen_quit()) return -1;                                    /* port: the input ended */
    return 0;
}

/* B9D3(pair, n): one credit for n ticks; -1 when a key ended it */
static int credit_B9D3(uint16_t pair, uint16_t n) {
    int skip = 0;                                                    /* [bp-2] */
    const char *title = (const char *)&g_ds[W(pair)], *names = (const char *)&g_ds[W(pair + 2)];
    /* b9e0..ba08: 8C9F(names, 8C58(title), 0, 159, 99) (8C58 pops the title, the names stay pushed as 8C9F's first
       argument), the rect copied to [bp-0Ah] */
    ui_box_t r = ui_text_rect(names, ui_text_advance(title), 0, 0x9F, 0x63);
    setW(0x9B69, 1);                                                 /* ba09 */
    nv_text_8BE4(title, 0x9F, 0x2F);                                 /* ba1c */
    setW(0x9B69, 0);                                                 /* ba1f */
    nv_text_8BE4(names, 0x9F, r.y0);                                 /* ba32 */
    setW(0x9AFA, W(0x6346));                                         /* ba35 */
    g_shown = 0;
    nv_t_start_00D7(1);                                              /* ba3f */
    while (nv_t_since_0183(1) < n) {                                 /* ba42 */
        if (poll_B8B7()) { skip = -1; break; }                       /* ba4e */
        nv_tick();                                                   /* port: one tick per pass */
    }
    nv_fill_7E74(r.x0, 0x2F, r.x1, r.y1);                            /* ba5a..ba67: 7E74(x0, 47, x1, y1) */
    g_shown = 0;
    return skip;
}

/* B9A1: the frame, the 18 pairs DS:6A6E.. (216 ticks each; a key ends the list), then the last pair DS:6AB6 (360 ticks) */
void credits_B9A1(void) {
    nv_target_screen_680F();                                         /* port: the screen is the target (it is, after BF6A) */
    province_frame30e6(0x54, 0x28);                                  /* b9a2..b9aa */
    g_shown = 0;
    for (uint16_t si = 0x6A6E; si < 0x6AB6; si = (uint16_t)(si + 4)) /* b9ad..b9c4 */
        if (credit_B9D3(si, 0xD8) == -1) break;                      /* b9ba: inc ax; je b9c6 */
    credit_B9D3(0x6AB6, 0x168);                                      /* b9c6..b9ce */
}
