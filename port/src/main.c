/* advciv GAMEDIR [--original] [--headless-frame OUT.png]
   The port's program. So far: the start-up the title menu needs (title_boot), the title menu (0x4F184) and, for a new
   game, the setup screens (title_newgame: 0x4DD50, 0x4EAE0).
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
#include "game.h"

static const char *s_frame_path;
static int s_pointer_x = -1, s_pointer_y = -1;
static const char *s_dump_path;
static uint32_t s_seed;
static int s_use_seed;

/* --click X Y (repeatable, headless): each is a press for one input poll, then a release; after the last one the
   frame is saved once the game has polled input a few more times (its screen is drawn before it waits) */
enum { MAX_CLICKS = 32, SETTLE_POLLS = 4 };
static int s_clicks[MAX_CLICKS][2], s_nclicks, s_click, s_pressed, s_idle;

static void save_frame_and_exit(void);

static void script_hook(void)
{
    if (s_click < s_nclicks) {   /* per click: 2 polls released (the game first waits for a release, 0x775F0),
                                    2 polls pressed (its click wait), then released again */
        int x = s_clicks[s_click][0], y = s_clicks[s_click][1];
        fg_test_mouse(x, y, s_pressed >= 2 && s_pressed < 4);
        if (++s_pressed == 5) {
            s_pressed = 0;
            s_click++;
        }
        return;
    }
    if (s_frame_path != NULL && ++s_idle >= SETTLE_POLLS)   /* with only --dump-ds the program runs on */
        save_frame_and_exit();
}

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
        else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            s_seed = (uint32_t)strtoul(argv[++i], NULL, 16);
            s_use_seed = 1;
        }
        else if (strcmp(argv[i], "--dump-ds") == 0 && i + 1 < argc)
            s_dump_path = argv[++i];
        else if (strcmp(argv[i], "--click") == 0 && i + 2 < argc && s_nclicks < MAX_CLICKS) {
            s_clicks[s_nclicks][0] = atoi(argv[++i]);
            s_clicks[s_nclicks][1] = atoi(argv[++i]);
            s_nclicks++;
        }
        else if (argv[i][0] != '-' && gamedir == NULL)
            gamedir = argv[i];
        else {
            fprintf(stderr, "usage: advciv GAMEDIR [--original] [--headless-frame OUT.png [--pointer X Y] [--click X Y]...]\n");
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
    if (plat_init("Advanced Civilization", s_frame_path != NULL || s_dump_path != NULL ? PLAT_HEADLESS : 0) != 0)
        return 1;
    fg_init();
    fg_set_gamedir(gamedir);
    if (s_frame_path != NULL || s_dump_path != NULL)
        fg_set_headless(1);
    rc = title_boot(gamedir);
    if (rc != 0) {
        fprintf(stderr, "advciv: start-up failed (%d)\n", rc);
        plat_shutdown();
        return 1;
    }
    if (s_frame_path != NULL || s_nclicks > 0)   /* start-up polls the keyboard too; the menu's first poll is its click
                                                    wait (0x4F6CE) */
        fg_set_input_hook(s_nclicks > 0 ? script_hook : save_frame_and_exit);
    for (;;) {   /* main 0x111A2..0x113FE: the menu, and for a new game the setup screens; CANCEL returns to 0x11168 */
        rc = title_menu();
        if (rc != -0x45)
            break;
        if (title_newgame() != 0) {
            rc = game_load(s_seed, s_use_seed);       /* main 0x114CF..0x1184A, up to the game loop 0x2EFA0 */
            if (s_dump_path != NULL && ds_dump(s_dump_path) != 0)
                fprintf(stderr, "advciv: cannot write %s\n", s_dump_path);
            fprintf(stderr, "advciv: new game loaded (%d), seed %08X; the game itself is not ported yet\n", rc,
                    (unsigned)D32(0xCAC5Cu));
            plat_shutdown();
            return 0;
        }
        title_menu_again();
    }
    fprintf(stderr, "advciv: the title menu returned %d (the game itself is not ported yet)\n", rc);
    plat_shutdown();
    return 0;
}
