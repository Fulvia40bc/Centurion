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

int main(int argc, char **argv) {
    /* --original (first argument): keep the original game's bugs, for byte-exact comparison with DOSBox */
    for (;;) {
        if (argc > 1 && strcmp(argv[1], "--original") == 0) { g_original_bugs = 1; argv++; argc--; continue; }
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
