#include "platform.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "state.h"
#include "assets.h"

void game_run(void);

int game_dump_screen(const char *name, const char *outpath);
int game_dump_frame(const char *outpath, const char *screen, const char *save);
int game_end_turn(const char *in_path, const char *out_path);
int game_save_game(const char *in_path, const char *out_path);
void game_play(void);
void game_init(void);

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

int main(int argc, char **argv) {
    /* --original (first argument): keep the original game's bugs, for byte-exact comparison with DOSBox */
    for (;;) {
        if (argc > 1 && strcmp(argv[1], "--original") == 0) { g_original_bugs = 1; argv++; argc--; continue; }
        if (argc > 2 && strcmp(argv[1], "--orders") == 0) { g_orders = argv[2]; battle_orders_hook = orders_hook; argv += 2; argc -= 2; continue; }
        if (argc > 2 && strcmp(argv[1], "--choices") == 0) { g_choices = argv[2]; battle_choices = choices_next; argv += 2; argc -= 2; continue; }
        break;
    }
    /* headless screen dump: centurion --dump-screen NAME out.bin */
    if (argc > 1 && strcmp(argv[1], "--play") == 0) {
        if (platform_init() != 0) return 1;
        game_init();
        game_play();
        platform_shutdown();
        return 0;
    }
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
