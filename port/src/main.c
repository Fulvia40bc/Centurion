#include "platform.h"
#include <string.h>

void game_run(void);

int game_dump_screen(const char *name, const char *outpath);
int game_dump_frame(const char *outpath, const char *screen, const char *save);
void game_play(void);
void game_init(void);

int main(int argc, char **argv) {
    /* headless screen dump: centurion --dump-screen NAME out.bin */
    if (argc > 1 && strcmp(argv[1], "--play") == 0) {
        if (platform_init() != 0) return 1;
        game_init();
        game_play();
        platform_shutdown();
        return 0;
    }
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
