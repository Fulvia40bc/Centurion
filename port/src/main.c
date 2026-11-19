/* advciv GAMEDIR [--original] [--headless-frame OUT.png]
   The port's program. So far: the start-up the title menu needs (title_boot), the title menu (0x4F184); for a new
   game the setup screens (title_newgame: 0x4DD50, 0x4EAE0), the loading step and the nation choice, to main 0x11854;
   for RESTORE A SAVED GAME the restore (title_restore, save.c), to main 0x115B0. There it stops: the game loop is not
   ported yet.
   --headless-frame opens no window: it runs until the game first waits for input, saves the screen as an indexed PNG
   (the 6-bit DAC widened as the VGA shows it) and exits, for frame comparisons with DOSBox-X captures. With --dump-ds
   it saves the frame at the dump instead. --click X Y (repeatable) scripts the clicks; --seed HEX sets the clock seed
   a new game starts from; --dump-ds OUT.bin writes the data object at the stop. */
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
#include "save.h"
#include "map.h"
#include "panel.h"
#include "start.h"

static const char *s_frame_path;
static int s_pointer_x = -1, s_pointer_y = -1;
static const char *s_dump_path;
static uint32_t s_seed;
static int s_use_seed;
static int s_dump_early;

/* --click X Y (repeatable, headless): each is a press for one input poll, then a release; after the last one the
   frame is saved once the game has polled input a few more times (its screen is drawn before it waits) */
enum { MAX_CLICKS = 32 };
static int s_settle = 4;   /* --settle N: input polls after the last click before the frame is saved */
static int s_clicks[MAX_CLICKS][2], s_nclicks, s_click, s_pressed, s_idle;

static void save_frame_and_exit(void);

/* --map-state DUMP.bin (module MAP1's frame check, test hook): after a restore and game_continue, copy from a DOSBox-X
   data-object dump (MEMDUMPBIN 0188:2A9000 72C00, taken where the original waits on the restored game's screen) only
   the plain game state the map window draws from (map.c's list: scroll, highlights, in play, territory, area, city
   and nation records, current player, phase, detail level, land/sea mode; no heap handles), then redraw the map
   window as the game does (f3D52C(0, 0): clamp, map picture, borders, markers, cities, tokens, ships, mini-map).
   The frame is then compared with the DOSBox-X capture in the map window and the mini-map only. */
static const char *s_map_state;
static int s_press = -1, s_mouse_x = -1, s_mouse_y = -1, s_overview, s_through_start, s_in_press;
static const struct { uint32_t addr, len; } MAP_STATE[] = {
    { 0xF4884u, 8 },        /* scroll column, row */
    { 0xF6110u, 4 },        /* int16 min, max column */
    { 0xFFB60u, 0xA0 },     /* border highlight colours */
    { 0xFFC00u, 0xA0 },     /* token box colours */
    { 0xFFCA0u, 0xA0 },     /* ship box colours */
    { 0xEE16Cu, 0xA0 },     /* in play */
    { 0xE1638u, 0x15E0 },   /* territory records 35 x 160 */
    { 0xE2C18u, 0xE0 },     /* owner / city bytes 3 x 157 (+ padding to 0xE2CF7) */
    { 0xE31B8u, 0x2B65 },   /* area records 69 x 161 */
    { 0xF14CCu, 0xAE6 },    /* nation records 199 x 14 */
    { 0xF40F8u, 4 },        /* current player */
    { 0xF4102u, 1 },        /* phase */
    { 0xC1A34u, 1 },        /* detail level */
    { 0x108804u, 0x1C },    /* land/sea mode, the last move (0x108808..0x10881C, UNDO MOVE) */
    { 0xF06CCu, 4 },        /* movement: source chosen */
    { 0x109E10u, 6 },       /* movement: the selected territory, int16 [0x109E14] */
    { 0xF09E0u, 2 },        /* int16 the count buttons' value */
    { 0xF09B4u, 6 },        /* the PBEM move log (handle, 0 outside PBEM) and its int16 counter */
    { 0xF8E44u, 0x32 },     /* the dialog helpers' saved enabled bytes */
    { 0x107690u, 2 },       /* int16 zoomed (the dispatcher's DETAIL and ZOOM test it) */
    { 0xEE168u, 1 },        /* the background picture of the game screen (f3CD6C, the status panel) */
    { 0xF06B0u, 0x10 },     /* int16[8] the nation of each player button */
    { 0xF40ECu, 4 },        /* players */
    { 0xF40F0u, 4 },        /* player count (the overview's zoom picture) */
    { 0xEE26Eu, 0x782 },    /* the overview's point arrays (tokens, cities, ships) */
};

static int map_state_draw(const char *path)
{
    static uint8_t dump[0x72C00];
    FILE *f = fopen(path, "rb");
    size_t i, n;

    if (f == NULL) {
        fprintf(stderr, "advciv: cannot read %s\n", path);
        return -1;
    }
    n = fread(dump, 1, sizeof dump, f);
    fclose(f);
    if (n != sizeof dump) {
        fprintf(stderr, "advciv: %s is not a data-object dump (%u bytes)\n", path, (unsigned)n);
        return -1;
    }
    for (i = 0; i < sizeof MAP_STATE / sizeof MAP_STATE[0]; i++)
        memcpy(DP(MAP_STATE[i].addr), dump + (MAP_STATE[i].addr - 0xB0000u), MAP_STATE[i].len);
    {   /* the panel dialog 0xF596C (int16 count, 20-byte records up to the fade buffer 0xF5D58): its label pointers
           (+0x08) are run-time addresses in a DOSBox-X dump; DOS/4GW put the data object at 0x2A9000 (link + 0x1F9000) */
        uint32_t r, cnt;
        memcpy(DP(0xF596Cu), dump + (0xF596Cu - 0xB0000u), 0xF5D58u - 0xF596Cu);
        cnt = D16(0xF596Cu);
        for (r = 0; r < cnt && 0xF596Eu + 20u * r + 20u <= 0xF5D58u; r++) {
            uint32_t at = 0xF596Eu + 20u * r + 8u, v = D32(at);
            if (v >= 0x2A9000u && v < 0x2A9000u + 0x72C00u)
                setD32(at, v - 0x1F9000u);
        }
    }
    setD8(0xF52D0u, 0xFF);   /* the port's background buffer holds the restore's picture: let f3CD6C reload */
    if (s_overview)          /* --overview: the ZOOM view of the current player (f3E374, as case 0x25 draws it) */
        f3E374(S32(0xF40F8u));
    else if (s_press >= 0) {   /* --press ID [--mouse X Y]: the button the player clicked next, through the dispatcher */
        if (s_mouse_x >= 0) {
            setD32(0xE1628u, (uint32_t)s_mouse_x);
            setD32(0xE162Cu, (uint32_t)s_mouse_y);
        }
        s_in_press = 1;      /* a screen this opens may wait for clicks: the input hook stops there (script_hook) */
        s_idle = 0;
        fprintf(stderr, "advciv: f3FB94(0x%X) = %d\n", (unsigned)s_press, f3FB94(s_press));
        s_in_press = 0;
    } else
        f3D52C(0, 0);
    return 0;
}

static void script_hook(void)
{
    if (s_click < s_nclicks) {   /* per click: 2 polls released (the game first waits for a release, 0x775F0),
                                    2 polls pressed (its click wait), then released again */
        int x = s_clicks[s_click][0], y = s_clicks[s_click][1];
        if (x < 0) {             /* --wait N: N polls with no button down (the pointer where the last click was) */
            if (++s_pressed >= y) {
                s_pressed = 0;
                s_click++;
            }
            return;
        }
        fg_test_mouse(x, y, s_pressed >= 2 && s_pressed < 4);
        if (++s_pressed == 5) {
            s_pressed = 0;
            s_click++;
        }
        return;
    }
    if (s_in_press && ++s_idle >= s_settle) {   /* the pressed button's screen waits for input: dump and frame there */
        if (s_dump_path != NULL && ds_dump(s_dump_path) != 0)
            fprintf(stderr, "advciv: cannot write %s\n", s_dump_path);
        fprintf(stderr, "advciv: the screen of button 0x%X waits for input; stopped there\n", (unsigned)s_press);
        if (s_frame_path != NULL)
            save_frame_and_exit();
        exit(0);
    }
    if (s_frame_path != NULL && s_dump_path == NULL && ++s_idle >= s_settle)   /* with --dump-ds the program runs
                                                                                     on and saves the frame at the dump */
        save_frame_and_exit();
}

static int save_frame(void)
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
    return rc;
}

static void save_frame_and_exit(void)
{
    exit(save_frame() != 0);
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
        else if (strcmp(argv[i], "--map-state") == 0 && i + 1 < argc)
            s_map_state = argv[++i];
        else if (strcmp(argv[i], "--wait") == 0 && i + 1 < argc && s_nclicks < MAX_CLICKS) {
            s_clicks[s_nclicks][0] = -1;              /* a script step of N polls without a click */
            s_clicks[s_nclicks][1] = atoi(argv[++i]);
            s_nclicks++;
        }
        else if (strcmp(argv[i], "--settle") == 0 && i + 1 < argc)
            s_settle = atoi(argv[++i]);
        else if (strcmp(argv[i], "--through-start") == 0)
            s_through_start = 1;                      /* new game: go on through the starting territories to 0x1188C */
        else if (strcmp(argv[i], "--overview") == 0)
            s_overview = 1;
        else if (strcmp(argv[i], "--press") == 0 && i + 1 < argc)
            s_press = (int)strtol(argv[++i], NULL, 0);
        else if (strcmp(argv[i], "--mouse") == 0 && i + 2 < argc) {
            s_mouse_x = atoi(argv[++i]);
            s_mouse_y = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--dump-at-1184F") == 0 || strcmp(argv[i], "--dump-early") == 0)
            s_dump_early = 1;                         /* new game: stop before the nation choice (the loading check);
                                                         restore: stop at 0x115B0 (the S1 check) */
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
        if (rc == -0x46) {                            /* RESTORE A SAVED GAME: main 0x11557 */
            if (title_restore()) {                    /* 0x115A9; the game goes on at 0x115B0 */
                if (!s_dump_early)                    /* --dump-early: stop at 0x115B0 (the S1 check) */
                    (void)game_continue(0, 0);        /* 0x115B0..0x1177C; returns -1: main goes on at 0x1188C */
                if (s_map_state != NULL && map_state_draw(s_map_state) != 0)
                    return 1;
                if (s_dump_path != NULL && ds_dump(s_dump_path) != 0)
                    fprintf(stderr, "advciv: cannot write %s\n", s_dump_path);
                if (s_dump_path != NULL && s_frame_path != NULL)
                    (void)save_frame();
                fprintf(stderr, "advciv: stopped at main 0x%X after a restore, seed %08X; the game itself is not "
                        "ported yet\n", s_dump_early ? 0x115B0u : 0x1188Cu, (unsigned)D32(0xCAC5Cu));
                plat_shutdown();
                return 0;
            }
            title_menu_again();                       /* cancelled: back to 0x11168 */
            continue;
        }
        if (rc != -0x45)
            break;
        if (title_newgame() != 0) {
            rc = game_load(s_seed, s_use_seed);       /* main 0x114CF..0x1184A */
            if (!s_dump_early)
                f2EFA0();                             /* 0x1184F: the nation choice; returns to 0x11854 */
            if (!s_dump_early && s_through_start)
                start_slice();                        /* 0x11854..0x11884: the starting territories (START) */
            if (s_dump_path != NULL && ds_dump(s_dump_path) != 0)
                fprintf(stderr, "advciv: cannot write %s\n", s_dump_path);
            if (s_dump_path != NULL && s_frame_path != NULL)   /* the screen at the dump, as run_setup.sh captures it */
                (void)save_frame();
            fprintf(stderr, "advciv: stopped at main 0x%X (%d), seed %08X; the game itself is not ported yet\n",
                    s_dump_early ? 0x1184Fu : s_through_start ? 0x1188Cu : 0x11854u, rc, (unsigned)D32(0xCAC5Cu));
            plat_shutdown();
            return 0;
        }
        title_menu_again();
    }
    fprintf(stderr, "advciv: the title menu returned %d (the game itself is not ported yet)\n", rc);
    plat_shutdown();
    return 0;
}
