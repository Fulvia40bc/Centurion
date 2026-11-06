#include "platform.h"
#include <string.h>

void game_run(void);

int game_dump_screen(const char *name, const char *outpath);

int main(int argc, char **argv) {
    /* headless screen dump: centurion --dump-screen NAME out.bin */
    for (int i = 1; i + 1 < argc; i++)
        if (strcmp(argv[i], "--dump-screen") == 0)
            return game_dump_screen(argv[i + 1], argv[i + 2]);
    if (platform_init() != 0) return 1;
    game_run();
    platform_shutdown();
    return 0;
}
