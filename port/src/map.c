/* The map screen and the main loop of the EXE (segment 0823), Claude 2026-11-09. Literal translations from
 * disasm/EXE_RUNTIME.clean.asm; docs/GAMELOOP_NOTES.md sections 1-2 (with its verifier corrections).
 *
 *   map_loop      0x07EF..0x09B9  the map loop in main 0x079D (with the start-up tail 0x07C5..0x07EE: fill, 494A(0))
 *   map_build     0x494A          the map screen: MAP image, flood fill of the Roman provinces, buttons, icons
 *     year_button 0x4A24, end_turn_button 0x4AA6, fleet_icons 0x2AF8, legion_icons 0x423A, army_icons 0x53BC,
 *     flood fill 0x22CC / 0x22EA / 0x22E2, sprite blit 0x8E62
 *   zone_at       0x6C67          zone under a point, run-length rows DS:3A96, the Asterix village (0xFE) message
 *   map_startup   resident overlay 0x0146..0x0152 (VGA colours) and 0x019E..0x01D5 (INIT sprite descriptors)
 *
 * Not ported here (as screen.c): the mouse cursor (0x1D55 / 0x1D8A / 0x1CCE), the music driver (0x5D73's loading and the
 * driver calls of 0x5D29 / 0x5D20; their [3EEE] writes are done) and the disk swap prompts (0x1E21). Fleet select / move
 * (0x2C6F, 0x2F5A, 0x2B40) and the victory sequence (section 4 DF58, not disassembled) are not ported: a message box
 * says so and the DS is left as it is.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "screen.h"
#include "state.h"
#include "assets.h"
#include "legions.h"

static const char *dstr(uint16_t a) { return (const char *)&g_ds[a]; }

/* ---- pictures: the MAP image (MMAP archive) and the INIT sprite sheet (MINIT archive) ---- */
static uint8_t g_mapimg[SCREEN_W * SCREEN_H];   /* MAP.CBM through its colour table: what 0x8790 puts down */
static int g_have_map;
static uint8_t *g_init_pix;                      /* INITDAT.LZW unpacked: the 4-bit pixels the descriptors point into */
static size_t g_init_npix;
static uint8_t g_init_hdr[14][14];               /* INITHDR.LZW: 14 descriptors of 14 bytes */
static int g_init_n;

/* MAP.CBM from MMAP.DAT, opaque, through the LUT of its header (the port's verified map picture, game.c) */
static void map_image_load(void) {
    if (g_have_map) return;
    img_entry_t *imgs = malloc(256 * sizeof(img_entry_t));
    if (!imgs) return;
    int n = assets_scan_images(".", imgs, 256);
    for (int i = 0; i < n; i++) {
        if (strcmp(imgs[i].name, "MAP.CBM") != 0) continue;
        int w, h, xo, yo, trans, lut = 0;
        uint8_t *px = assets_decode_cbm_file(imgs[i].dat, &imgs[i], &w, &h, &xo, &yo, &trans, &lut);
        if (!px) break;
        const uint8_t *t = assets_lut(lut ? lut : 0);
        if (!t) t = assets_lut(0);
        memset(g_mapimg, 0, sizeof(g_mapimg));
        for (int y = 0; y < h && y < SCREEN_H; y++)
            for (int x = 0; x < w && x < SCREEN_W; x++) g_mapimg[y * SCREEN_W + x] = t[px[y * w + x] & 0xF];
        free(px);
        g_have_map = 1;
        break;
    }
    free(imgs);
    if (!g_have_map) fprintf(stderr, "map: MAP.CBM not found (run in the game folder)\n");
}

/* The INIT sheet: header entries (raw 14-byte descriptors) and the unpacked pixels */
static void init_sheet_load(void) {
    if (g_init_pix) return;
    sprite_ent_t ents[16];
    int n = 0;
    if (assets_load("MINIT.DIR", "MINIT.DAT") != 0 || assets_load_sheet("INITHDR.LZW", "INITDAT.LZW", ents, &n, &g_init_pix, &g_init_npix) != 0) {
        fprintf(stderr, "map: INITHDR.LZW / INITDAT.LZW not found in MINIT.DAT (run in the game folder)\n");
        g_init_pix = NULL; g_init_npix = 0;
        return;
    }
    if (n > 14) n = 14;
    g_init_n = n;
    for (int i = 0; i < n; i++) {            /* back to the file's 14-byte form <HhhBBBBI */
        uint8_t *d = g_init_hdr[i];
        int bpr = (ents[i].w + 1) / 2;
        d[0] = (uint8_t)ents[i].w; d[1] = (uint8_t)(ents[i].w >> 8);
        d[2] = (uint8_t)ents[i].xhot; d[3] = (uint8_t)(ents[i].xhot >> 8);
        d[4] = (uint8_t)ents[i].yhot; d[5] = (uint8_t)(ents[i].yhot >> 8);
        d[6] = (uint8_t)bpr; d[7] = (uint8_t)ents[i].h; d[8] = (uint8_t)ents[i].trans; d[9] = (uint8_t)ents[i].lut;
        d[10] = (uint8_t)ents[i].off; d[11] = (uint8_t)(ents[i].off >> 8); d[12] = (uint8_t)(ents[i].off >> 16); d[13] = (uint8_t)(ents[i].off >> 24);
    }
}

/* The start-up writes this piece needs, on a DS that has not been through them (a DS from state_init / state_load):
 *  - resident overlay 0x0146..0x0152 (VGA, [9B26] == 2): [6344] = 74h, [6346] = 1Ah, [6348] = 10Ch (panel, box and text
 *    colours; the static OVL0 image has 3 / 7 / 7). The port is always VGA. Same values as every DOSBox snapshot.
 *  - 0x019E..0x01D5: the INIT sheet's 14 descriptors are loaded (0x655B) and their addresses stored through the pointer
 *    table DS:6350 (C0E2, C1D6, C108, C10C, C10A, C110, C0F0, C0F2, C0F4, C0E6, C0E8, C0EA, C0EC, C0EE): the map
 *    icons. Done only when those pointers are 0 (static DS); the descriptors go where the snapshots have them,
 *    DS:CE48 + 14*i (heap, beyond the static image), with the offset word +0x0A = the offset in INITDAT and the segment
 *    word +0x0C = 371Bh as in the snapshots (the port reads the pixels from g_init_pix, not through that segment).
 *  - [3E40] = 3E2E "Click to continue" (ui_mouse_init, the start-up's mouse branch).
 *  - the font record DS:9B56..9B5F and the graphics mode [9B26] (a new game had them at 0).
 * Running it on a DOSBox DS image changes nothing (the same values are already there). */
void map_startup(void) {
    setW(0x6344, 0x74); setW(0x6346, 0x1A); setW(0x6348, 0x10C);
    /* section 1 0xB1A1 (OVL0 file 0xC1): rep movsw 5 words DS:A05C -> DS:9B56, the font record: three table pointers,
       line height [9B5C] = 8 and line gap [9B5D] = 3. Every box laid out by line (legion box and list, move header,
       fleet list, number fields) reads them; at 0 the rows collapse onto each other. */
    memcpy(&g_ds[0x9B56], &g_ds[0xA05C], 10);
    /* EXE 0x7DCE: mov [9B26], ax, the graphics mode the start-up chose; 2 = VGA in every DOSBox capture. The province
       strips (0x70BA: sky colour 0x17, not 0x0B) and the screen copies (6 = Tandy paths) read it. */
    setW(0x9B26, 2);
    ui_mouse_init();
    init_sheet_load();
    map_image_load();
    int empty = 1;
    for (int i = 0; i < 14; i++) if (W(W(0x6350 + 2 * i))) empty = 0;
    if (!empty || !g_init_n) return;
    for (int i = 0; i < g_init_n; i++) {
        uint16_t d = (uint16_t)(0xCE48 + 14 * i);
        memcpy(&g_ds[d], g_init_hdr[i], 10);
        setW((uint16_t)(d + 0x0A), (uint16_t)(g_init_hdr[i][10] | g_init_hdr[i][11] << 8));
        setW((uint16_t)(d + 0x0C), 0x371B);
        setW(W(0x6350 + 2 * i), d);
    }
}

/* 0x8E62 sprite(spr, x, y): the descriptor at DS:spr {w, xhot, yhot, bytes per row, rows, transparent nibble, colour
   table, far pointer to the pixels}; drawn at (x - xhot, y - yhot), clipped to the target (817C against C0DA..C0E0 =
   the whole 320x200 picture); the transparent nibble is skipped, the others go through the colour table. spr 0 draws
   nothing. The pixels are read from the INIT sheet (every descriptor this file draws comes from it). The descriptor's
   pixel offsets count from 2 bytes before the first pixel byte of INITDAT (entry 0 has offset 2): VERIFIED, at offset - 2
   the three enemy ships of scr_newgame_map.bin match 177/177 pixels each at (anchor - hot), at the offset itself 63. */
static void sprite8e62(uint8_t *fb, uint16_t spr, int x, int y) {
    if (!spr || !g_init_pix) return;
    int w = W(spr), x0 = x - SW(spr + 2), y0 = y - SW(spr + 4);
    int bpr = B(spr + 6), rows = B(spr + 7);
    uint8_t trans = B(spr + 8);
    const uint8_t *lut = assets_lut(B(spr + 9));
    size_t off = W(spr + 0x0A);
    if (!lut || off < 2) return;
    off -= 2;
    for (int r = 0; r < rows; r++) {
        int dy = y0 + r;
        if (dy < 0 || dy >= SCREEN_H) continue;
        for (int c = 0; c < w; c++) {
            int dx = x0 + c;
            if (dx < 0 || dx >= SCREEN_W) continue;
            size_t k = off + (size_t)r * bpr + (size_t)c / 2;
            if (k >= g_init_npix) continue;
            uint8_t nib = (c & 1) ? (g_init_pix[k] & 0xF) : (g_init_pix[k] >> 4);
            if (nib == trans) continue;
            fb[dy * SCREEN_W + dx] = lut[nib];
        }
    }
}

/* ---- the flood fill (0x22CC alloc of a 500-entry ring of {x, y}, 0x22EA fill, 0x22E2 free) ---- */
/* 0x83BD get_pixel(x, y) of the work buffer. Outside the picture the original reads past its row table; the port
   answers 0xFFFF there (never the colour being replaced, so the fill stops). */
static int getpix(int x, int y) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) return 0xFFFF;
    return g_wrk[y * SCREEN_W + x];
}
/* 0x22EA fill(x, y, c): [9AFA] = c; old = pixel (x, y); a span fill driven by a ring of 500 seeds (head [bp-0Ah],
   tail [bp-8]): each seed walks left while the pixel is old (x < 319), then paints right while the pixel is old
   (x < 319), queuing the first old pixel of each run above and below. Seeds with x >= 319 or y >= 199 are dropped. */
static void fill22ea(int x, int y, int c) {
    static int16_t q[500][2];                     /* [A69A] */
    setW(0x9AFA, (uint16_t)c);
    int old = getpix(x, y);
    if (old == c) return;
    unsigned head = 0, tail = 1;
    q[0][0] = (int16_t)x; q[0][1] = (int16_t)y;
    while (tail != head) {                        /* 0x2326 */
        x = q[head][0]; y = q[head][1];
        head = (head + 1) % 500;
        if (x >= 0x13F || y >= 0xC7) continue;
        while (getpix(x, y) == old) {             /* 0x2370 */
            if (x >= 0x13F) break;
            x--;
        }
        int up = -1, down = -1, yu = y - 1, yd = y + 1;   /* [bp-2], [bp-4], [bp-0Ch], [bp-0Eh] */
        for (int si = x + 1; ; si++) {            /* 0x2398 */
            if (getpix(si, y) != old) break;
            if (si >= 0x13F) break;
            g_wrk[y * SCREEN_W + si] = (uint8_t)c;   /* 0x82DE in [9AFA] */
            if (getpix(si, yu) == old) {
                if (up) { up = 0; q[tail][0] = (int16_t)si; q[tail][1] = (int16_t)yu; tail = (tail + 1) % 500; }
            } else up = -1;
            if (getpix(si, yd) == old) {
                if (down) { down = 0; q[tail][0] = (int16_t)si; q[tail][1] = (int16_t)yd; tail = (tail + 1) % 500; }
            } else down = -1;
        }
    }
}

/* ---- buttons and icons ---- */
/* 0x3134 box(x0, y0, x1, y1, fill): ui.c's ui_box draws it; the DS writes of 0x3134 are done here: [9AFA] = fill (0x3145,
   only when fill != -1), 8 (0x3157), 0 (0x31F5), 0Fh (0x3211), so [9AFA] = 0Fh afterwards. */
static void box3134(int x0, int y0, int x1, int y1, int fill) {
    ui_box(g_wrk, x0, y0, x1, y1, fill);
    if (fill != -1) setW(0x9AFA, (uint16_t)fill);
    setW(0x9AFA, 8);
    setW(0x9AFA, 0);
    setW(0x9AFA, 0x0F);
}
/* 0x4A24 year_button(turn): v = 275 - turn; AC60 = itoa(|v|, 10) + (v > 0 ? " BC" (3CBE) : " AD" (3CC2)); box 3134
   (3C4A..3C50, [6346]); [9B69] = 15; 8BE4(AC60, (X0 + X1) / 2, Y0 + 4). (Turn 275 shows "0 AD", as the original.) */
static void year_button(uint16_t turn) {
    int cx = (int16_t)(SW(0x3C4E) + SW(0x3C4A)) / 2;
    int16_t v = (int16_t)(0x113 - turn);
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", (int16_t)(v < 0 ? -v : v));   /* 9FDC itoa of the 16-bit |v| */
    strcpy((char *)&g_ds[0xAC60], buf);
    strcat((char *)&g_ds[0xAC60], dstr(v > 0 ? 0x3CBE : 0x3CC2));  /* 9DF2 */
    box3134(SW(0x3C4A), SW(0x3C4C), SW(0x3C4E), SW(0x3C50), SW(0x6346));
    setW(0x9B69, 15);
    ui_text_block_attr(g_wrk, cx, SW(0x3C4C) + 4, dstr(0xAC60), W(0x9B69));
}
/* 0x4AA6 end_turn_button: box 3134 (3C42..3C48, [6346]); [9B69] = 15; 899C("End Turn" 3CC6, X0 + 5, Y0 + 4) */
static void end_turn_button(void) {
    box3134(SW(0x3C42), SW(0x3C44), SW(0x3C46), SW(0x3C48), SW(0x6346));
    setW(0x9B69, 15);
    ui_text_attr(g_wrk, SW(0x3C42) + 5, SW(0x3C44) + 4, dstr(0x3CC6), W(0x9B69));
}
/* zone anchor DS:1AB8 + 4z {x, y} */
static int ax_of(unsigned z) { return SW(0x1AB8 + 4 * z); }
static int ay_of(unsigned z) { return SW(0x1ABA + 4 * z); }
/* 0x2AF8: every fleet record (DS:110E + 12k, k 0..5) with +8 != FFh at the anchor of its zone (+6); Roman fleets 0-2
   with sprite [C0F0], the enemy's 3-5 with [C0F4] */
static void fleet_icons(void) {
    for (uint16_t si = 0x1116; si < 0x115E; si += 0x0C) {
        if (B(si) == 0xFF) continue;
        uint8_t z = B(si - 2);
        sprite8e62(g_wrk, si < 0x113A ? W(0xC0F0) : W(0xC0F4), ax_of(z), ay_of(z));
    }
}
/* 0x423A: every legion record (DS:1918 + 20L, L 0..11) with +0x0D != FFh at its zone (+0x0C): [C108] on land
   (zone < 27), [C10A] at sea */
static void legion_icons(void) {
    for (uint16_t si = 0x1924; si < 0x1A14; si += 0x14) {
        if (B(si + 1) == 0xFF) continue;
        uint8_t z = B(si);
        sprite8e62(g_wrk, z < 0x1B ? W(0xC108) : W(0xC10A), ax_of(z), ay_of(z));
    }
}
/* 0x53BC: every army record (DS:C112 + 14i, i 0..10) with +8 != 0 at its zone (+1): [C110] on land, [C0F4] at sea */
static void army_icons(void) {
    for (uint16_t si = 0xC113; si < 0xC1AD; si += 0x0E) {
        if (W(si + 7) == 0) continue;
        uint8_t z = B(si);
        sprite8e62(g_wrk, z < 0x1B ? W(0xC110) : W(0xC0F4), ax_of(z), ay_of(z));
    }
}

/* 0x5D29 music_play(m): 5D73(m) first (loads music m into a free slot of the table DS:A6B0 {handle, loaded, id} when no
   loaded slot has it: file + driver, not ported); then, when [3EEE] != m, every slot i 0..3 whose id DS:A6B3[4i] is m is
   started (072E, the driver: not ported) and [3EEE] = m. The port does the DS part (the table as the start-up left it). */
static void music_play(uint8_t m) {
    if (B(0x3EEE) == m) return;
    for (int i = 0; i < 4; i++)
        if (B(0xA6B3 + 4 * i) == m) B(0x3EEE) = m;
}

/* 0x494A map_build(z): 1D55(0) (cursor, not ported); the MAP picture (archive 5, loaded once: the port keeps it, the
   original tests [254]); mouse range x 0..311 (06E4), y 0..185 (0709); z != FFh: pointer to the anchor of z (066A);
   67E4 (draw into the work buffer); 8790 (the picture); 22CC; for each province k 0..26 with status +0x15 != 0:
   22EA(seed DS:3C52[k], colour 9); 22E2; 4A24([634C]); 4AA6; 2AF8; 423A; 53BC; 680F; 6973(0, 0, 319, 199) (work buffer
   to the screen); 5D29(1) (the map music: its DS part); 1D8A. */
void map_build(int z) {
    map_image_load();
    setW(0xBC, 0); setW(0xC0, 0x137);
    setW(0xBE, 0); setW(0xC2, 0xB9);
    if ((uint8_t)z != 0xFF) mouse_warp(ax_of((uint8_t)z), ay_of((uint8_t)z));
    memcpy(g_wrk, g_mapimg, sizeof(g_wrk));
    for (uint16_t si = 0x3C52, di = 0x5285; di <= 0x55C5; si += 4, di += 0x20)
        if (B(di) != 0) fill22ea(SW(si), SW(si + 2), 9);
    year_button(W(0x634C));
    end_turn_button();
    fleet_icons();
    legion_icons();
    army_icons();
    memcpy(g_scr, g_wrk, sizeof(g_scr));
    music_play(1);
    screen_present();
}

/* 0x6C67 zone_at(x, y): row = DS:[3A96 + 2y], run-length pairs {zone, length}; walk while the summed length dx < 319 and
   x >= dx (signed). Zone FEh (the Asterix village on Armorica's coast) shows DS:[57BC] in a message box and gives FFh.
   0..26 provinces, 27..36 seas, FFh none. */
static uint8_t zone_run(int x, int y) {
    uint16_t row = W(0x3A96 + 2 * y);
    int dx = 0;
    uint16_t i = 0;
    uint8_t z = 0xFF;                             /* [bp-3]: always set, the loop runs at least once */
    while (dx < 0x13F) {
        z = B(row + i); i++;
        dx += B(row + i); i++;
        if (!((int16_t)x >= dx)) break;
    }
    return z;
}
int zone_at(int x, int y) {
    uint8_t z = zone_run(x, y);
    if (z == 0xFE) { message_box(dstr(W(0x57BC))); z = 0xFF; }
    return z;
}
/* zone_at without the message (for the zone check in main.c) */
int zone_at_raw(int x, int y) { return zone_run(x, y); }

/* ---- the loop ---- */
/* 0x8238 in_rect(rect, y, x): inclusive, signed */
static int in_rect8238(uint16_t r, int x, int y) {
    return x >= SW(r) && x <= SW(r + 4) && y >= SW(r + 2) && y <= SW(r + 6);
}
/* 0x5D20 music_stop: [3EEE] = FFh (current music), then the driver (not ported) */
static void music_stop(void) { B(0x3EEE) = 0xFF; }

/* 0x08EF..0x0919: the right button on a zone: legions there (0x3BAF "Select Legion", 0x3F23, 0x3EA3, 0x382A) */
static void right_button(int btn, uint8_t z) {
    if (btn != 2) return;
    uint8_t l = legions_select(z, 0);
    legions_close_box();
    if (l >= 12) return;
    legions_move_header(l);
    legions_move(l);
}

/* main 0x07C5..0x07EE (the end of the start-up: [9AFA] = 0, 7E74 fill (0, 0)-(319, 199), 494A(0), 1CCE cursor shown)
   and the map loop 0x07EF..0x09B9. Returns where the original calls 0x0EE7 exit_game(0). The port also leaves when the
   window is closed or the script has ended (screen_quit()). */
void map_loop(void) {
    int quit = 0;                                         /* di */
    setW(0x9AFA, 0);
    memset(g_scr, 0, sizeof(g_scr));
    map_build(0);
    while (!quit && !screen_quit()) {                     /* 0x09A7 */
        int btn = wait_input();                           /* 0x07EF, [bp-5] */
        if (btn != 0) {
            int x = SW(0xB6), y = SW(0xB8);
            if (in_rect8238(0x3C42, x, y)) end_turn();    /* 0x080F */
            else if (in_rect8238(0x3C4A, x, y)) {         /* 0x0828: the year button = Game Control */
                music_stop();
                quit = game_control_menu();
                map_build(0xFF);
            } else {
                uint8_t z = (uint8_t)zone_at(x, y);       /* 0x0839, [bp-4] */
                if (z < 0x1B) {
                    if (B(0x5285 + 32 * z) != 0) {        /* a Roman province */
                        B(0xC080) = z;
                        if (btn == 1) {
                            music_stop();
                            if (z == 0) rome_screen(); else province_screen(z);
                        } else right_button(btn, z);
                    } else zone_info(z);                  /* 0x087F */
                } else if (z != 0xFF) {                   /* a sea: the last fleet record in z wins */
                    uint8_t f = 0xFF;                     /* [bp-3] */
                    for (uint16_t si = 0x1114, k = 0; si < 0x115C; si += 0x0C, k++)
                        if (B(si) == z && B(si + 2) != 0xFF) f = (uint8_t)k;
                    if (f == 0xFF) zone_info(z);
                    else if (btn != 1) right_button(btn, z);
                    else if (f >= 3) zone_info(z);
                    else                                  /* 0x08C9: 2C6F select, 3F23, 2F5A info, 2B40 move */
                        message_box("Fleet orders (Select Fleet, moving a fleet)\nare not ported yet.");
                }
            }
        } else {                                          /* 0x091B: toupper([C07F]) (9DDC) */
            int key = toupper(B(0xC07F));
            switch (key) {
            case 0x1B: music_stop(); quit = game_control_menu(); map_build(0xFF); break;   /* 0x0828 */
            case 'E': end_turn(); break;                                                    /* 0x080F */
            case 'F': fleet_list(); break;                                                  /* 0x2FD6 */
            case 'L': legion_list(); break;                                                 /* 0x4280 */
            case 'M': map_build(0xFF); break;                                               /* 0x0830 */
            case 'P': province_list(); break;                                               /* 0x69CC */
            case 'Q': if (yes_no(dstr(0x172))) quit = -1; break;                            /* "Really Quit?" */
            default: break;
            }
        }
        /* 0x096E: no province with status != 0 left: the empire has fallen */
        int any = 0;
        for (uint16_t bx = 0x5285; bx < 0x55E5; bx += 0x20) if (B(bx) != 0) any = -1;
        if (!any) { message_box(dstr(0x17F)); quit = -1; }
        wait_release();                                   /* 0x35DB */
        if (B(0x5971) == 8) {                             /* rank Caesar: 1E21 disk check, section 4 DF58 victory */
            quit = -1;
            message_box("The victory sequence\nis not ported yet.");
        }
    }
}
