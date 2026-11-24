/* The game's sound backend (port, Claude 22 Nov): snd.h's calls on ail.c (Mistral's TASK 22/22a: the AIL 3.03
   subset on the mixer, the XMIDI sequencer and the music synths) and the platform's audio device.
   - Headless runs (--headless-frame, --dump-ds, --dump-every: demo_regress, the DOSBox-X compares) open no device,
     load no synth, play nothing and write no settings: ail.c then runs its "not live" rules (a play-once sample is
     done at once, a looping one reports playing until ended), so nothing waits on real time and the game's data
     object is the same as with sound (the handles 0xF6274..0xF6284 are masked: by_design.txt; demo_regress records
     them as they are).
   - The music engine (Erik's ruling of 18 Nov, OVERVIEW §7.2) plays its own set whatever set the data object names
     (title.c's boot_music_set, from MDI.INI as the original; newgame.c's f7B854 asks snd_music_image for the song):
     - 's', stereo FM, the default (23 Nov): the game's SBPRO2.MDI and EHGM2FM2.OPL from the game folder on a Nuked
       OPL3 (fm_synth.c);
     - 'f', FM: the game's ADLIB.MDI and EHGM2FM2.AD the same way (mono OPL2);
     - 'g', General MIDI: FluidSynth (gm_synth.c) with the first *.sf2 in the game folder (name order), else
       GeneralUser-GS.sf2 next to advciv.exe (the player's zip); without one the music is silent and the game goes on.
     An FM set whose files are missing or unusable plays General MIDI instead.
   - The choice: the settings file advciv.ini next to advciv.exe ("music=stereo-fm|fm|gm", "fm-swap=0|1"; the game
     folder keeps only the original's files), then main.c's --music and --fm-swap for that run, then the music line
     of the Preferences screen (menu2.c), which switches the engine at once and saves the file. */
#include "snd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ail.h"
#include "catalog.h"
#include "platform.h"
#include "snd_ail.h"

static int g_headless;
static char g_choice = 's';                           /* the set chosen: 's' stereo FM, 'f' FM, 'g' General MIDI */
static int g_swap;                                    /* FM: the sides mirrored against DOSBox-X's (this run) */
static int g_swap_file;                               /* the settings file's fm-swap, which a save writes back */
static int g_live;                                    /* the audio device is open */
static uint8_t *g_ct5;                                /* ACIV.CT5, read once (not headless): the songs the engine plays */
static catalog g_cat;
static int g_cat_ok;
static volatile int g_up;                             /* the device may call ail_render (set under the lock) */

static void fill(void *user, int16_t *out, int frames)
{
    (void)user;
    if (g_up)
        ail_render(out, frames);
    else
        memset(out, 0, (size_t)frames * 2u * sizeof *out);
}

static void lock(int on) { plat_audio_lock(on); }

/* a whole file, or NULL */
static uint8_t *load(const char *name, size_t *len)
{
    FILE *f = fopen(name, "rb");
    uint8_t *b = NULL;
    long n = -1;

    if (f == NULL)
        return NULL;
    if (fseek(f, 0, SEEK_END) == 0 && (n = ftell(f)) > 0 && fseek(f, 0, SEEK_SET) == 0) {
        b = malloc((size_t)n);
        if (b != NULL && fread(b, 1, (size_t)n, f) != (size_t)n) {
            free(b);
            b = NULL;
        }
    }
    fclose(f);
    if (b != NULL)
        *len = (size_t)n;
    return b;
}

/* ---- the settings file ------------------------------------------------------------------------------------------ */

static void settings_path(char *p, size_t n)
{
    snprintf(p, n, "%sadvciv.ini", plat_exe_dir());
}

static const char *set_name(char set)
{
    return set == 'f' ? "fm" : set == 'g' ? "gm" : "stereo-fm";
}

void snd_ail_load_settings(void)
{
    char p[1024], line[128];
    FILE *f;

    settings_path(p, sizeof p);
    f = plat_fopen(p, "r");                           /* the exe's folder is a UTF-8 path */
    if (f == NULL)
        return;                                       /* no file: the defaults (stereo FM, the sides as DOSBox-X) */
    while (fgets(line, sizeof line, f) != NULL) {
        char key[32], val[32];

        if (sscanf(line, " %31[^= ] = %31s", key, val) != 2)
            continue;
        if (strcmp(key, "music") == 0) {
            if (strcmp(val, "stereo-fm") == 0)
                g_choice = 's';
            else if (strcmp(val, "fm") == 0)
                g_choice = 'f';
            else if (strcmp(val, "gm") == 0)
                g_choice = 'g';
        } else if (strcmp(key, "fm-swap") == 0) {
            g_swap = g_swap_file = strcmp(val, "1") == 0;
        }
    }
    fclose(f);
}

static void save_settings(void)
{
    char p[1024];
    FILE *f;

    if (g_headless)
        return;
    settings_path(p, sizeof p);
    f = plat_fopen(p, "w");
    if (f == NULL) {
        fprintf(stderr, "advciv: cannot write %s: the music choice is not kept\n", p);
        return;
    }
    fprintf(f, "; Advanced Civilization port settings (the original's own settings stay in ACIV.CFG)\n");
    fprintf(f, "music=%s\n", set_name(g_choice));
    fprintf(f, "fm-swap=%d\n", g_swap_file);          /* the file's own: a run's --fm-swap is not kept */
    fclose(f);
}

void snd_ail_set_music(char set, int swap)
{
    if (set != 0)
        g_choice = set == 'f' || set == 'g' ? set : 's';
    if (swap >= 0)
        g_swap = swap != 0;
}

/* ---- the engines -------------------------------------------------------------------------------------------- */

static void sf2_find(char *sf2, size_t n, int *have)
{
    *have = 0;
    if (plat_first_match(".", "*.sf2", sf2, (int)n) == 0)
        *have = 1;
    else {
        FILE *f;
        snprintf(sf2, n, "%sGeneralUser-GS.sf2", plat_exe_dir());
        f = plat_fopen(sf2, "rb");
        if (f != NULL) {
            fclose(f);
            *have = 1;
        }
    }
    if (!*have)
        fprintf(stderr, "advciv: no SoundFont (a .sf2 in the game folder or GeneralUser-GS.sf2 next to advciv.exe):"
                        " the General MIDI music is silent\n");
}

/* the FM set's driver and bank: init = 1 at the start-up (ail_init_fm), 0 for a switch (ail_music_fm) */
static int start_fm(char set, int init)
{
    const char *drv = set == 'f' ? "ADLIB.MDI" : "SBPRO2.MDI", *bank = set == 'f' ? "EHGM2FM2.AD" : "EHGM2FM2.OPL";
    size_t mdi_len = 0, gtl_len = 0;
    uint8_t *mdi = load(drv, &mdi_len), *gtl = load(bank, &gtl_len);
    int ok = 0;

    if (mdi != NULL && gtl != NULL)
        ok = init ? ail_init_fm(g_live ? lock : NULL, mdi, mdi_len, gtl, gtl_len, g_swap)
                  : ail_music_fm(mdi, mdi_len, gtl, gtl_len, g_swap);
    free(mdi);                                        /* fm_synth keeps its own copies */
    free(gtl);
    if (!ok)
        fprintf(stderr, "advciv: %s or %s missing from the game folder, or not usable: General MIDI music instead\n",
                drv, bank);
    return ok;
}

static snd_ops ops;

static void startup(void)
{
    char sf2[1024];
    int have = 0;

    if (g_headless) {
        ail_init(NULL, NULL);
        return;
    }
    g_live = plat_audio_open(fill, NULL) == 0;
    {   /* the music catalog, read apart from the game's catalog calls (they write file names into its data) */
        size_t n = 0;
        g_ct5 = load("ACIV.CT5", &n);
        if (g_ct5 != NULL && cat_open(&g_cat, g_ct5, n) == 0)
            g_cat_ok = 1;
    }
    ops.music_set = 'g';
    if (g_choice != 'g' && start_fm(g_choice, 1))
        ops.music_set = g_choice;
    else {
        sf2_find(sf2, sizeof sf2, &have);
        ail_init(g_live ? lock : NULL, have ? sf2 : NULL);
    }
    if (g_live) {
        plat_audio_lock(1);
        g_up = 1;
        plat_audio_lock(0);
        ail_set_live(1);
    }
}

static char music_choice(void) { return g_choice; }

static char music_switch(char set)
{
    char sf2[1024];
    int have = 0;

    snd_ail_set_music(set, -1);
    save_settings();
    if (g_headless)
        return ops.music_set = g_choice;              /* no engine: the set is only named */
    if (g_choice == ops.music_set)
        return ops.music_set;                         /* that engine plays already: nothing to change */
    if (g_choice != 'g' && start_fm(g_choice, 0))
        ops.music_set = g_choice;
    else if (ops.music_set != 'g') {                  /* General MIDI, chosen or as the fallback (unless it plays) */
        sf2_find(sf2, sizeof sf2, &have);
        ail_music_gm(have ? sf2 : NULL);
        ops.music_set = 'g';
    }
    return ops.music_set;
}

static void shutdown(void)
{
    if (g_up) {
        ail_set_live(0);
        plat_audio_lock(1);
        g_up = 0;                                     /* the device keeps running until plat_shutdown: silence */
        plat_audio_lock(0);
    }
    ail_shutdown();
    g_cat_ok = 0;
    free(g_ct5);
    g_ct5 = NULL;
}

static const char *last_error(void) { return ail_last_error(); }
static void seq_volume(int32_t h, int v) { ail_set_sequence_volume(h, v, 0); }

/* the engine plays its own set: "smap__1c.xmi" / "fmap__1c.xmi" / "gmap__1c.xmi" share their names but the first
   letter (music.md), which becomes the engine's; NULL when the catalog was not read (headless) or the file is not
   there (the game's own block is played then) */
static const uint8_t *music_image(const char *name, size_t *len)
{
    char g[16];
    const cat_entry *e;

    if (!g_cat_ok || strlen(name) >= sizeof g)
        return NULL;
    strcpy(g, name);
    if (g[0] == 'f' || g[0] == 'F' || g[0] == 's' || g[0] == 'S' || g[0] == 'g' || g[0] == 'G')
        g[0] = ops.music_set;
    e = cat_find(&g_cat, g);
    if (e == NULL || cat_data(&g_cat, e) == NULL)
        return NULL;
    *len = e->length;
    return cat_data(&g_cat, e);
}

/* boot_sound allocates each of the three slots and AIL_init_samples it at once (0x10BA8.., D) */
static int32_t alloc_sample(void)
{
    int32_t h = ail_allocate_sample();
    if (h != 0)
        ail_init_sample(h);
    return h;
}

static const snd_ops ops_init = {
    startup, shutdown, alloc_sample, ail_allocate_sequence,
    ail_set_sample_file, ail_set_sample_volume, ail_set_sample_loop_count, ail_start_sample, ail_end_sample,
    ail_sample_status, ail_active_sample_count,
    ail_init_sequence, ail_start_sequence, ail_stop_sequence, ail_resume_sequence, ail_end_sequence,
    ail_sequence_status, seq_volume, ail_set_sequence_loop_count, last_error, music_image, 'g',
    music_choice, music_switch, ail_start_sequence_paused
};

void snd_ail_attach(int headless)
{
    g_headless = headless;
    ops = ops_init;
    if (headless)
        ops.music_set = g_choice;                     /* no engine: the set is only named (the music line shows it) */
    snd_attach(&ops);
}
