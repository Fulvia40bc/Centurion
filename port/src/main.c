/* advciv GAMEDIR [--original] [--headless-frame OUT.png]
   The port's program. So far: the start-up the title menu needs (title_boot) and the title menu (0x4F184).
   --headless-frame opens no window: it runs until the game first waits for input, saves the screen as an indexed PNG
   (the 6-bit DAC widened as the VGA shows it) and exits, for frame comparisons with DOSBox-X captures. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fg.h"
#include "palette.h"
#include "platform.h"
#include "png_write.h"
#include "state.h"
#include "ui.h"

static const char *s_frame_path;
static int s_pointer_x = -1, s_pointer_y = -1;

static void save_frame_and_exit(void)
{
    uint8_t rgb[256][3];
    int rc;

    pal_dac_to_rgb(fg_dac(), rgb);
    if (s_pointer_x >= 0) {   /* the screen with Fastgraph's pointer at a given place, as a DOSBox-X capture has it */
        static uint8_t frame[SCREEN_W * SCREEN_H];
        fg_frame_with_pointer(frame, s_pointer_x, s_pointer_y);
        rc = png_write_indexed(s_frame_path, frame, SCREEN_W, SCREEN_H, (const uint8_t (*)[3])rgb);
    } else
        rc = png_write_indexed(s_frame_path, fg_screen(), SCREEN_W, SCREEN_H, (const uint8_t (*)[3])rgb);
    if (rc != 0)
        fprintf(stderr, "advciv: cannot write %s\n", s_frame_path);
    exit(rc != 0);
}

int main(int argc, char **argv)
{
    const char *gamedir = NULL;
    int i, rc;

    plat_console();
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--original") == 0)
            g_original = 1;
        else if (strcmp(argv[i], "--headless-frame") == 0 && i + 1 < argc)
            s_frame_path = argv[++i];
        else if (strcmp(argv[i], "--pointer") == 0 && i + 2 < argc) {
            s_pointer_x = atoi(argv[++i]);
            s_pointer_y = atoi(argv[++i]);
        }
        else if (argv[i][0] != '-' && gamedir == NULL)
            gamedir = argv[i];
        else {
            fprintf(stderr, "usage: advciv GAMEDIR [--original] [--headless-frame OUT.png]\n");
            return 2;
        }
    }
    if (gamedir == NULL)
        gamedir = ".";
    if (plat_chdir(gamedir) != 0) {   /* the original runs in its folder: saves, aciv.cfg, pal.pal are relative */
        fprintf(stderr, "advciv: cannot change to the game folder %s\n", gamedir);
        return 1;
    }
    if (s_frame_path != NULL && s_frame_path[0] != '/' && s_frame_path[0] != '\\' && s_frame_path[1] != ':')
        fprintf(stderr, "advciv: note: %s is relative to the game folder now\n", s_frame_path);
    if (plat_init("Advanced Civilization", s_frame_path != NULL ? PLAT_HEADLESS : 0) != 0)
        return 1;
    fg_init();
    fg_set_gamedir(gamedir);
    if (s_frame_path != NULL)
        fg_set_headless(1);
    rc = title_boot(gamedir);
    if (rc != 0) {
        fprintf(stderr, "advciv: start-up failed (%d)\n", rc);
        plat_shutdown();
        return 1;
    }
    if (s_frame_path != NULL)   /* start-up polls the keyboard too; the menu's first poll is its click wait (0x4F6CE) */
        fg_set_input_hook(save_frame_and_exit);
    rc = title_menu();
    fprintf(stderr, "advciv: the title menu returned %d (the game itself is not ported yet)\n", rc);
    plat_shutdown();
    return 0;
}
