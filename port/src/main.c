#include "platform.h"

void game_run(void);

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    if (platform_init() != 0) return 1;
    game_run();
    platform_shutdown();
    return 0;
}
