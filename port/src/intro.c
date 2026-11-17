/* The intro, OVL0 section 1 (file 0x0000-0x09A0, loaded at 0823:B0E0), Claude 2026-11-17. Translated literally from
 * disasm/OVL_SECT1.asm after docs/INTRO_NOTES.md (sections 2, 4, 7); addresses in the comments.
 *
 *   intro_run  section 1 B18A's slice around the intro: b25b music_load(0) ("cn1", CENT), b265 B694, b26b music_unload(0)
 *   B694       the intro: the she-wolf (ROMULUS.CBM) and the music, the legend (8 lines), the world (WORLD.LZW) with the
 *              title (TITLE.CBM) and the sea's colour cycle, the 11 credits (B970), the closing text
 *   B970       the first 11 credit pairs DS:6A6E..6A96, 288 ticks each (credit_B9D3 and the poll B8B7 are credits.c's)
 *   B8CF       DAC A2..A6 back to the five colours DS:6ABA (VGA)
 *
 * Timeline with no key (72.8 Hz ticks of timer 0 from the music start; D): 0 the she-wolf; 504 + 144k the legend line k
 * at (5, 5 + 11k) in colour 6; 2664 the world and the title at (4, 74), the sea cycling; 3024 the world again, the frame
 * 30E6 (84, 40) and the credits; then the frame again and "It is 400 years after the\nFounding, ..." centred on
 * (159, 99) in white for 720 ticks. Skipping (keys only, 03AA, as 1990): the first key jumps to the closing box (after a
 * key in the she-wolf or legend phase the world, title and credits are never shown and the box lands on the she-wolf);
 * a second key ends it.
 *
 * WHEN (main.c play_game, INTRO_NOTES 7): after map_startup (which made the start-up writes B18A had made before B694: the
 * font record, [6346] = 1Ah for the frame, [9B26] = 2 for the cycle) and before map_loop, for an interactive start only:
 * no --script and no --load (G, Erik's call: 1990 has no load before the intro). `--intro` runs it in any start, so a
 * --script run can drive and shoot it ("wait N" / "key C" / "shot NAME" lines; a "click" line waits for the map). The
 * intro writes no game state: [9AFA], [9B69], the cycle words DS:6AE2..6AE6, the timer slots DS:42..4D, the music words
 * ([3EEE], the slot table DS:A6B0), the cursor stack byte DS:A674 (6973's push / pop), the drawing target DS:C0D0..C0E1
 * (67E4 / 680F copy C1DA / 9AB6 into it) and, in a window, the mouse words DS:B2 / B6 / B8 (credits_poll reads the
 * mouse, a port addition: 1990's B694 reads keys only), all rewritten before they are read (the map code sets the
 * target, map_build(0) warps the pointer); no dice. So every scripted check is unchanged. With --intro and --load, a DS
 * whose arrow is visible ([686] = FFFF) has it hidden around the intro (intro_run, port-only: 1990 shows it only after).
 *
 * Not ported (as the rest of the port): the copy-protection question B466 (Erik's decision; its one dice roll and the
 * archive-name decryption with it), the EGA path (WORLD.CBM, absent from MINIT). The music calls (load, play CN1, stop,
 * unload) are music.c's: in a window game the title theme plays from b75f (2026-11-17).
 *
 * 1990 BUGS: fix 1 = B8FE's colour cycle mod 4 (credits.c, shared; --original keeps mod 4: DAC A6 never moves). Nothing
 * else found. Kept as 1990 (not bugs, G): the early-key path above; "400 years after the Founding" while the game starts
 * in 275 BC (no text of the game gives a founding year).
 *
 * Easter eggs / hidden text (kept): the credits name the tools ("Soft-ICE & MagicCV\nby Nu-Mega", "Plink-86plus by
 * Polytron", "MSC v5.1, v6.0 & MASM v5.1\nby Microsoft", "Deluxe Paint II for IBM and\nDeluxe Paint II & III\nfor
 * Amiga\nby Dan Silva & Steve Shaw"). B466's name key "kenny" stays in the DS, unused, as the question is skipped.
 *
 * PORT: one tick (nv_tick) per pass of each wait, as credits.c; the window's events are read each pass and the screen is
 * shown when it changed; a closed window or a script that ran out counts as a key (credits.c). The window's system
 * pointer is hidden, as 1990's mouse reset (0632, INT 33h fn 0) leaves the driver's pointer hidden; the game's arrow
 * comes after the intro (B18A 1DF5, map_loop's 1CCE). Pictures: race_gfx.c's pic_t pattern (1273 + 8E62 + 86A0),
 * repeated here.
 *
 * NOT VERIFIED against DOSBox: no capture of the intro exists. D, and checked by headless --intro shots.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "state.h"
#include "screen.h"
#include "assets.h"
#include "platform.h"
#include "naval_int.h"

void province_frame30e6(int x, int y);  /* province.c: 0x30E6 on the screen */
int  credits_poll(int cycle);           /* credits.c: B8B7 (cycle 1) or 03AA alone (0); -1 = a key */
int  credit_B9D3(uint16_t pair, uint16_t n);
void credits_redraw(void);

static const char *dstr(uint16_t a) { return (const char *)&g_ds[a]; }

/* ---- 1273 / 8E62 / 86A0 of a CBM member (race_gfx.c's pic_t) ---- */
typedef struct { uint8_t *px; int w, h, xo, yo, trans, lut; } pic_t;
static void pic_free(pic_t *p) { free(p->px); memset(p, 0, sizeof(*p)); }
static int pic_load(pic_t *p, const char *lower) {
    char up[16]; size_t i;
    pic_free(p);
    for (i = 0; lower[i] && i < sizeof(up) - 1; i++) up[i] = (char)(lower[i] >= 'a' && lower[i] <= 'z' ? lower[i] - 32 : lower[i]);
    up[i] = 0;
    int k = assets_find(up);
    if (k < 0) { fprintf(stderr, "intro: %s missing\n", up); return 0; }
    size_t rl = 0;
    uint8_t *raw = assets_decode(k, &rl);
    uint8_t *px = assets_decode_cbm(k, &p->w, &p->h, &p->lut);
    if (!raw || !px || rl < 10) { free(raw); free(px); memset(p, 0, sizeof(*p)); fprintf(stderr, "intro: %s unreadable\n", up); return 0; }
    p->xo = (int16_t)(raw[2] | raw[3] << 8); p->yo = (int16_t)(raw[4] | raw[5] << 8); p->trans = raw[8];
    free(raw);
    p->px = px;
    return 1;
}
/* 8E62: at (x - xhot, y - yhot), the transparent nibble skipped, the colour table DS:60C0 + 16 * lut, clipped to the
   target's rect C0DA..C0E0 */
static void pic_draw(const pic_t *p, int x, int y) {
    if (!p->px) return;
    x -= p->xo; y -= p->yo;
    for (int r = 0; r < p->h; r++) {
        int Y = y + r;
        if (Y < SW(0xC0DC) || Y > SW(0xC0E0) || Y < 0 || Y >= SCREEN_H) continue;
        for (int c = 0; c < p->w; c++) {
            int X = x + c, n = p->px[r * p->w + c] & 15;
            if (n == p->trans || X < SW(0xC0DA) || X > SW(0xC0DE) || X < 0 || X >= SCREEN_W) continue;
            nv_fb[Y * SCREEN_W + X] = B(0x60C0 + 16 * p->lut + n);
        }
    }
}

/* the waits: one tick per pass; -1 = a key */
static int wait_key(int cycle) {
    if (credits_poll(cycle)) return -1;
    nv_tick();
    return 0;
}

/* B970: the frame, then the pairs DS:6A6E..6A96 for 288 ticks each; -1 when a key ended one */
static int credits_B970(void) {
    int di = 0;
    province_frame30e6(0x54, 0x28);                                  /* b974..b97c */
    credits_redraw();
    for (uint16_t si = 0x6A6E; si < 0x6A9A; si = (uint16_t)(si + 4)) /* b97f..b99a */
        if ((di = credit_B9D3(si, 0x120)) == -1) break;              /* b989..b991 */
    return di;
}

/* B8CF (VGA): 84BB (the retrace wait: nothing), then DAC W(6AD8 + 2k) = the colour DS:6ABA + 6k, k 0..4 */
static void colours_B8CF(void) {
    if (W(0x9B26) != 2) return;                                      /* b8d1 */
    for (uint16_t si = 0x6ABA, di = 0x6AD8; di < 0x6AE2; si = (uint16_t)(si + 6), di = (uint16_t)(di + 2))
        screen_dac(W(di), W(si), W(si + 2), W(si + 4));              /* b8e7..b8f1: 01B6 */
    credits_redraw();
}

/* B694: the intro; -1 when a picture could not be loaded (1990: "Could Not Open Data File.", exit_game(1)) */
static int intro_B694(void) {
    int skip = 0;                                                    /* si, kept in [bp-2] across the legend lines */
    pic_t rom = { 0 }, tit = { 0 };                                  /* [bp-2Ch], [bp-3Ah] */
    int ok = assets_load("MINIT.DIR", "MINIT.DAT") == 0;             /* b69e: 0F7B(4): "minit" */
    if (ok) {
        ok &= pic_load(&rom, dstr(0x6A17));                          /* b6bb: 1273("romulus.cbm") */
        ok &= pic_load(&tit, dstr(0x6A23));                          /* b6cc: 1273("title.cbm") */
        nv_target_work_67E4();                                       /* b6d1 */
        /* b6d4: VGA, 0F0C("world.lzw") straight into the work buffer (the result is not tested: b6ee jmp b713); the EGA
           path (WORLD.CBM, b6f0..b710) is not ported */
        int k = assets_find("WORLD.LZW");
        size_t n = 0;
        uint8_t *w = k >= 0 ? assets_decode(k, &n) : NULL;
        if (w) memcpy(g_wrk, w, n < sizeof(g_wrk) ? n : sizeof(g_wrk));
        else fprintf(stderr, "intro: WORLD.LZW missing\n");
        free(w);
        nv_target_screen_680F();                                     /* b717: 11A9 archive_close; b71a: 680F */
    }
    if (!ok) {                                                       /* b71d */
        pic_free(&rom); pic_free(&tit);
        message_box(dstr(0x6A41));                                   /* b725: "Could Not Open Data File." */
        return -1;                                                   /* b72c: exit_game(1) */
    }
    if (!screen_headless()) platform_pointer(0);                     /* port: the pointer hidden (header) */
    music_stop_5D20();                                               /* b72f */
    setW(0x9AFA, 0);                                                 /* b732 */
    nv_fill_7E74(0, 0, 0x13F, 0xC7);                                 /* b744 */
    pic_draw(&rom, 0, 0);                                            /* b74f: 8E62(romulus, 0, 0) */
    pic_free(&rom);                                                  /* b756: 86A0 */
    credits_redraw();
    nv_t_reset_00F8();                                               /* b759 */
    music_play_5D29(0);                                              /* b75f: "cn1" */
    while (!skip && nv_t_since0_01AE() < 0x1F8)                      /* b764..b778: until 504 ticks, 03AA only */
        skip = wait_key(0);
    if (!skip) {                                                     /* b77d */
        setW(0x9B69, 6);                                             /* b7c0 */
        int y = 5;                                                   /* [bp-6] */
        for (uint16_t di = 0x6A5C; di < 0x6A6C; di = (uint16_t)(di + 2)) {   /* b7cb..b81e: the 8 legend lines */
            nv_t_start_00D7(1);                                      /* b7d4 */
            ui_text_attr(g_scr, 5, y, dstr(W(di)), W(0x9B69));       /* b7e0: 899C(line, 5, y) */
            credits_redraw();
            y += B(0x9B5C) + B(0x9B5D);                              /* b7e3..b7f0 */
            while (!skip && nv_t_since_0183(1) < 0x90)               /* b7fb..b813: 144 ticks a line, 03AA */
                skip = wait_key(0);
        }
        while (!skip && nv_t_since0_01AE() < 0xA68)                  /* b820..b839: until 2664 ticks */
            skip = wait_key(0);
        if (!skip) {                                                 /* b781 */
            nv_restore_6973(0, 0, 0x13F, 0xC7);                      /* b794: the world */
            pic_draw(&tit, 0, 0x40);                                 /* b7a2: 8E62(title, 0, 64): hot (-4, -10) */
            credits_redraw();
            while (!skip && nv_t_since0_01AE() < 0xBD0)              /* b7a5..b7be: until 3024 ticks, B8B7 */
                skip = wait_key(1);
        }
    }
    pic_free(&tit);                                                  /* b83f: 86A0 */
    if (!skip) {                                                     /* b842 */
        nv_restore_6973(0, 0, 0x13F, 0xC7);                          /* b852: the world again (the title erased) */
        credits_B970();                                              /* b855: the result is ignored */
    }
    province_frame30e6(0x54, 0x28);                                  /* b860: also after a key */
    ui_box_t r = ui_text_rect(dstr(W(0x6A6C)), 0, 0, 0x9F, 0x63);    /* b877: 8C9F(DS:6619, 0, 0, 159, 99) */
    setW(0x9B69, 0x0F);                                              /* b885 */
    nv_text_8BE4(dstr(W(0x6A6C)), 0x9F, r.y0);                       /* b896: 8BE4(text, 159, [bp-14h]) */
    credits_redraw();
    nv_t_reset_00F8();                                               /* b899 */
    while (nv_t_since0_01AE() < 0x2D0)                               /* b89c..b8a9: 720 ticks or a key, B8B7 */
        if (wait_key(1)) break;
    colours_B8CF();                                                  /* b8ab */
    music_stop_5D20();                                               /* b8ae */
    return 0;
}

/* B18A's slice: b25b music_load(0) (inside its CENT branch), b265 B694, b26b music_unload(0); -1 = B694 failed */
int intro_run(void) {
    /* PORT: 1990 runs B694 before 1DF5 sets the arrow up ([686] = 0 all through it); a --load DS (--intro with --load, a
       test path) can have it visible ([686] = FFFF in every DOSBox image), and 6973's pop would draw it on the intro */
    int shown = W(0x686) != 0;
    if (shown) cursor_push_1D55(0);
    music_load_5D73(0);                                              /* b25b: "cn1" */
    int r = intro_B694();
    music_unload_5E1A(0);                                            /* b26b */
    if (shown) cursor_pop_1D8A();
    return r;
}
