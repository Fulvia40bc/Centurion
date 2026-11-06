#include "platform.h"
#include <string.h>

void game_run(void);

int game_dump_screen(const char *name, const char *outpath);
int game_dump_frame(const char *outpath);

int main(int argc, char **argv) {
    /* headless screen dump: centurion --dump-screen NAME out.bin */
    for (int i = 1; i < argc; i++) {
        if (i + 1 < argc && strcmp(argv[i], "--dump-screen") == 0)
            return game_dump_screen(argv[i + 1], argv[i + 2]);
        if (i + 1 < argc && strcmp(argv[i], "--dump-frame") == 0)
            return game_dump_frame(argv[i + 1]);
    }
    if (platform_init() != 0) return 1;
    game_run();
    platform_shutdown();
    return 0;
}
