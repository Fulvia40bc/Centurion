#include "platform.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "state.h"
#include "assets.h"
#include "screen.h"

void game_run(void);

int game_dump_screen(const char *name, const char *outpath);
int game_dump_frame(const char *outpath, const char *screen, const char *save);
int game_end_turn(const char *in_path, const char *out_path);
int game_save_game(const char *in_path, const char *out_path);
void game_init(void);
void map_startup(void);                  /* map.c: the start-up writes the map needs (colours, INIT sprites) */
int  zone_at_raw(int x, int y);          /* map.c: 0x6C67 without the Asterix message */

/* --choices "F,T;F,T": formation/tactic for each battle in a headless run, in order */
static const char *g_choices;
static int choices_next(uint8_t *f, uint8_t *t) {
    unsigned a, b; int n = 0;
    if (!g_choices || sscanf(g_choices, "%u,%u%n", &a, &b, &n) != 2) return 0;
    *f = (uint8_t)a; *t = (uint8_t)b; g_choices += n;
    if (*g_choices == ';') g_choices++;
    fprintf(stderr, "battle: formation %u, tactic %u\n", a, b);
    return 1;
}

/* --orders "T:ops;T:ops": orders given in the orders mode after battle tick T, for replays. ops, separated by spaces:
   M = Release The Legion, R = Run Away (both leave the orders mode), uI = pick unit I (cursor at its cell), uI@X,Y = pick
   unit I by clicking cell X,Y, tX,Y = drag/cursor to cell X,Y, h = hold, m = cohort melee, r = cohort retreat, e = done
   with the unit. */
static const char *g_orders;
static int orders_hook(int tick) {
    const char *p = g_orders;
    while (p && *p) {
        int t, n = 0;
        if (sscanf(p, "%d:%n", &t, &n) != 1) return 0;
        p += n;
        const char *end = strchr(p, ';'); if (!end) end = p + strlen(p);
        if (t == tick) {
            fprintf(stderr, "orders at tick %d\n", tick);
            while (p < end) {
                unsigned i, x, y; int k = 0, ok;
                while (*p == ' ') p++;
                if (p >= end) break;
                if (*p == 'M') { battle_order_legion_melee(); p++; }
                else if (*p == 'R') { battle_order_legion_retreat(); p++; }
                else if (*p == 'u') {
                    if (sscanf(p, "u%u@%u,%u%n", &i, &x, &y, &k) == 3) ok = battle_cmd_begin((int)i, (uint8_t)x, (uint8_t)y);
                    else { sscanf(p, "u%u%n", &i, &k); ok = battle_cmd_begin((int)i, 0xFF, 0xFF); }
                    if (!ok) fprintf(stderr, "orders: unit %u can't take orders\n", i);
                    p += k ? k : 1;
                }
                else if (*p == 't' && sscanf(p, "t%u,%u%n", &x, &y, &k) == 2) { battle_cmd_target((uint8_t)x, (uint8_t)y); p += k; }
                else if (*p == 'h') { battle_cmd_hold(); p++; }
                else if (*p == 'm') { battle_cmd_melee(); p++; }
                else if (*p == 'r') { battle_cmd_retreat(); p++; }
                else if (*p == 'e') { battle_cmd_end(); p++; }
                else p++;
            }
        }
        p = *end ? end + 1 : end;
    }
    return 0;
}

/* Battles of a scripted run: the next "battle F T" line of the script, and its "orders" lines replayed by orders_hook */
static int script_choices(uint8_t *f, uint8_t *t) {
    if (!screen_script_choices(f, t)) return 0;
    g_orders = screen_script_orders();
    battle_orders_hook = g_orders ? orders_hook : NULL;
    return 1;
}

/* 0x075A's source: the BIOS tick count (18.2 Hz since midnight) */
static uint32_t bios_ticks(void) {
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    long s = lt ? lt->tm_hour * 3600L + lt->tm_min * 60L + lt->tm_sec : (long)(now % 86400);
    return (uint32_t)((double)s * 1193180.0 / 65536.0);
}

/* The game: main 0x079D in the port's order. --play = a new game (gamectl.c new_game, NEWGAME.LZW), --load SAVE = a .CSV
   save (state_load) or a 64 KB DS image / 640K snapshot ending in .bin (state_load_mem, dice included); then the map loop
   (map.c). --script FILE SHOTDIR runs it headless with the input from FILE (screen.c), shots into SHOTDIR; --ds-out FILE
   writes the DS image when the loop ends; --seed N uses N (decimal or 0x hex) as the BIOS tick count that 0x075A seeds the
   dice from, instead of the clock, so a --play or .CSV run can be repeated (a .bin load brings its own dice, DS:00C8).
   Run in the game folder. */
static int play_game(int argc, char **argv) {
    const char *load = NULL, *script = NULL, *shotdir = NULL, *dsout = NULL;
    int have_seed = 0;
    uint32_t seed = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--play") == 0) continue;
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) { seed = (uint32_t)strtoul(argv[++i], NULL, 0); have_seed = 1; continue; }
        if (strcmp(argv[i], "--load") == 0 && i + 1 < argc) { load = argv[++i]; continue; }
        if (strcmp(argv[i], "--script") == 0 && i + 2 < argc) { script = argv[i + 1]; shotdir = argv[i + 2]; i += 2; continue; }
        if (strcmp(argv[i], "--ds-out") == 0 && i + 1 < argc) { dsout = argv[++i]; continue; }
        fprintf(stderr, "unknown or incomplete option %s\n", argv[i]);
        return 1;
    }
    if (state_init("OVL0.OVL") != 0) { fprintf(stderr, "cannot read OVL0.OVL (run in the game folder)\n"); return 1; }
    rng_seed(have_seed ? seed : bios_ticks());                      /* 0x075A */
    if (screen_open(script != NULL) != 0) return 1;                 /* a script is always headless */
    if (load) {
        size_t n = strlen(load);
        int mem = n > 4 && (strcmp(load + n - 4, ".bin") == 0 || strcmp(load + n - 4, ".BIN") == 0);
        if ((mem ? state_load_mem(load) : state_load(load)) != 0) { fprintf(stderr, "cannot load %s\n", load); screen_close(); return 1; }
    } else if (new_game() != 0) { fprintf(stderr, "new game: cannot read NEWGAME.LZW\n"); screen_close(); return 1; }
    map_startup();
    if (script) { screen_script(script, shotdir); battle_choices = script_choices; }
    battle_ui_attach(1);
    legions_ui_install();
    map_loop();
    if (dsout) {
        FILE *f = fopen(dsout, "wb");
        if (!f || fwrite(g_ds, 1, 0x10000, f) != 0x10000) fprintf(stderr, "cannot write %s\n", dsout);
        if (f) fclose(f);
    }
    screen_close();
    return 0;
}

/* --zone-check: 0x6C67 on the static DS: every zone anchor DS:1AB8[z] must hit z, and the Asterix pixels (FEh) */
static int zone_check(void) {
    if (state_init("OVL0.OVL") != 0) return 1;
    int ok = 0, fe = 0, none = 0;
    for (int z = 0; z < 37; z++) {
        int r = zone_at(SW(0x1AB8 + 4 * z), SW(0x1ABA + 4 * z));
        if (r == z) ok++; else printf("anchor %d (%d,%d) -> %d\n", z, SW(0x1AB8 + 4 * z), SW(0x1ABA + 4 * z), r);
    }
    for (int y = 0; y < 200; y++)
        for (int x = 0; x < 320; x++) {
            int r = zone_at_raw(x, y);
            if (r == 0xFE) { fe++; printf("FE at (%d,%d)\n", x, y); }
            if (r == 0xFF) none++;
        }
    printf("anchors %d/37, FE pixels %d, FF pixels %d\n", ok, fe, none);
    return ok == 37 ? 0 : 1;
}

int main(int argc, char **argv) {
    /* --original (first argument): keep the original game's bugs, for byte-exact comparison with DOSBox */
    for (;;) {
        if (argc > 1 && strcmp(argv[1], "--original") == 0) { g_original_bugs = 1; argv++; argc--; continue; }
        if (argc > 2 && strcmp(argv[1], "--orders") == 0) { g_orders = argv[2]; battle_orders_hook = orders_hook; argv += 2; argc -= 2; continue; }
        if (argc > 2 && strcmp(argv[1], "--choices") == 0) { g_choices = argv[2]; battle_choices = choices_next; argv += 2; argc -= 2; continue; }
        break;
    }
    /* the game: --play | --load SAVE, [--script FILE SHOTDIR] [--ds-out FILE] [--seed N] (see play_game) */
    if (argc > 1 && (strcmp(argv[1], "--play") == 0 || strcmp(argv[1], "--load") == 0 || strcmp(argv[1], "--script") == 0 ||
                     strcmp(argv[1], "--seed") == 0))
        return play_game(argc, argv);
    if (argc > 1 && strcmp(argv[1], "--zone-check") == 0) return zone_check();
    if (argc > 3 && strcmp(argv[1], "--end-turn") == 0)
        return game_end_turn(argv[2], argv[3]);
    /* --battle-from-formation IN OUT army legion zone formation tactic: resume a battle captured at Choose Formation */
    if (argc > 8 && strcmp(argv[1], "--battle-from-formation") == 0) {
        if (state_load_mem(argv[2]) != 0) return 1;
        uint16_t r = battle_from_formation(atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), (uint8_t)atoi(argv[7]), (uint8_t)atoi(argv[8]));
        fprintf(stderr, "battle returned %04X\n", r);
        FILE *f = fopen(argv[3], "wb");
        if (!f) return 1;
        fwrite(g_ds, 1, 0x10000, f); fclose(f);
        return 0;
    }
    /* --battle IN OUT army legion zone formation tactic: the whole battle from a snapshot taken before it (run in the game folder) */
    if (argc > 8 && strcmp(argv[1], "--battle") == 0) {
        if (state_load_mem(argv[2]) != 0) return 1;
        size_t n = 0; uint8_t *tac = NULL; int k;
        if (assets_load("BATL.DIR", "BATL.DAT") == 0 && (k = assets_find("TACTICS.LZW")) >= 0) tac = assets_decode(k, &n);
        if (!tac) { fprintf(stderr, "TACTICS.LZW not found (run in the game folder)\n"); return 1; }
        uint16_t r = battle(atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), (uint8_t)atoi(argv[7]), (uint8_t)atoi(argv[8]), tac, n);
        fprintf(stderr, "battle returned %04X\n", r);
        FILE *f = fopen(argv[3], "wb");
        if (!f) return 1;
        fwrite(g_ds, 1, 0x10000, f); fclose(f);
        return 0;
    }
    /* --battle-frame SNAP ZONE OUT: draw the battle frame from a 640K snapshot taken during a battle (run in the game
       folder); re-sorts the sprite list and reports whether the sort matches the game's */
    if (argc > 4 && strcmp(argv[1], "--battle-frame") == 0) {
        if (state_load_mem(argv[2]) != 0) return 1;
        if (assets_load_luts("OVL0.OVL") != 0 || assets_load("BATL.DIR", "BATL.DAT") != 0) return 1;
        if (battle_gfx_load(atoi(argv[3])) != 0) return 1;
        uint8_t before[0x50];
        memcpy(before, &g_ds[0xAA02], sizeof(before));
        battle_sort_sprites();
        fprintf(stderr, "sprites %u, sort %s\n", W(0xA928),
                memcmp(before, &g_ds[0xAA02], 2u * W(0xA928)) ? "DIFFERS from the game's" : "same as the game's");
        static uint8_t fb[SCREEN_W * SCREEN_H];
        battle_draw(fb);
        FILE *f = fopen(argv[4], "wb");
        if (!f) return 1;
        fwrite(fb, 1, sizeof(fb), f); fclose(f);
        return 0;
    }
    /* --play-battle SNAP army legion zone [OUT]: fight a battle on screen from a DS image or 640K snapshot taken at
       (or before) the battle's start, e.g. tools/dosbox/results/battle/b11_entry.bin 7 0 0. Run in the game folder. */
    if (argc > 5 && strcmp(argv[1], "--play-battle") == 0) {
        if (state_load_mem(argv[2]) != 0) return 1;
        if (platform_init() != 0) return 1;
        uint16_t r = battle_play(atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
        platform_shutdown();
        fprintf(stderr, "battle returned %04X\n", r);
        if (argc > 6) { FILE *f = fopen(argv[6], "wb"); if (f) { fwrite(g_ds, 1, 0x10000, f); fclose(f); } }
        return 0;
    }
    /* --battle-shots SNAP army legion zone formation tactic tick PREFIX: the battle's screens as pictures, headless */
    if (argc > 9 && strcmp(argv[1], "--battle-shots") == 0) {
        if (state_load_mem(argv[2]) != 0) return 1;
        uint16_t r = battle_ui_shots(atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), atoi(argv[7]), atoi(argv[8]), argv[9]);
        fprintf(stderr, "battle returned %04X\n", r);
        return 0;
    }
    /* --report-shot DS L won rc ec rs es BG OUT: the battle report drawn over picture BG (64000 bytes, or "-") */
    if (argc > 10 && strcmp(argv[1], "--report-shot") == 0) {
        if (state_load_mem(argv[2]) != 0) return 1;
        static uint8_t bg[64000]; int have = 0;
        if (strcmp(argv[9], "-") != 0) { FILE *f = fopen(argv[9], "rb"); if (f) { have = fread(bg, 1, sizeof(bg), f) == sizeof(bg); fclose(f); } }
        battle_report_shot(atoi(argv[3]), atoi(argv[4]), (unsigned)atoi(argv[5]), (unsigned)atoi(argv[6]), (unsigned)atoi(argv[7]),
                           (unsigned)atoi(argv[8]), have ? bg : NULL, argv[10]);
        return 0;
    }
    /* --tactics-dump OUT: TACTICS.LZW decoded */
    if (argc > 2 && strcmp(argv[1], "--tactics-dump") == 0) {
        size_t n = 0; int k; uint8_t *tac = NULL;
        if (assets_load("BATL.DIR", "BATL.DAT") == 0 && (k = assets_find("TACTICS.LZW")) >= 0) tac = assets_decode(k, &n);
        if (!tac) return 1;
        FILE *f = fopen(argv[2], "wb"); if (!f) return 1; fwrite(tac, 1, n, f); fclose(f); return 0;
    }
    if (argc > 3 && strcmp(argv[1], "--save-game") == 0)
        return game_save_game(argv[2], argv[3]);
    for (int i = 1; i < argc; i++) {
        if (i + 2 < argc && strcmp(argv[i], "--dump-screen") == 0)
            return game_dump_screen(argv[i + 1], argv[i + 2]);
        if (i + 1 < argc && strcmp(argv[i], "--dump-frame") == 0) {
            const char *screen = NULL, *save = NULL;
            for (int j = i + 2; j < argc; j++) {
                if (strcmp(argv[j], "--save") == 0 && j + 1 < argc) { save = argv[j + 1]; j++; }
                else screen = argv[j];
            }
            return game_dump_frame(argv[i + 1], screen, save);
        }
    }
    if (platform_init() != 0) return 1;
    game_run();
    platform_shutdown();
    return 0;
}
