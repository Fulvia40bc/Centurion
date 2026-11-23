/* The game's sound backend (port, Claude 22 Nov): snd.h's calls on ail.c (Mistral's TASK 22/22a: the AIL 3.03
   subset on the mixer, the XMIDI sequencer and the GM synth) and the platform's audio device.
   - Headless runs (--headless-frame, --dump-ds, --dump-every: demo_regress, the DOSBox-X compares) open no device
     and load no SoundFont: ail.c then runs its "not live" rules (a play-once sample is done at once, a looping one
     reports playing until ended), so nothing waits on real time and the game's data object is the same as with
     sound (the handles 0xF6274..0xF6284 are masked: by_design.txt; demo_regress records them as they are).
   - The music engine is General MIDI for now (FluidSynth, gm_synth.c), so the songs play from the g set whatever
     set the data object names (newgame.c's f7B854 loads the g file for it). Erik's ruling of 18 Nov (three sets,
     default stereo FM, a Preferences line and a settings file) waits for the FM driver.
   - SoundFont: the first *.sf2 in the game folder (name order), else GeneralUser-GS.sf2 next to advciv.exe (the
     player's zip); without one the music is silent and the game goes on. */
#include "snd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ail.h"
#include "catalog.h"
#include "platform.h"
#include "snd_ail.h"

static int g_headless;
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

static void startup(void)
{
    char sf2[1024];
    int have_sf2 = 0, live = 0;

    if (!g_headless) {
        if (plat_first_match(".", "*.sf2", sf2, (int)sizeof sf2) == 0)
            have_sf2 = 1;
        else {
            FILE *f;
            snprintf(sf2, sizeof sf2, "%sGeneralUser-GS.sf2", plat_exe_dir());
            f = fopen(sf2, "rb");
            if (f != NULL) {
                fclose(f);
                have_sf2 = 1;
            }
        }
        if (!have_sf2)
            fprintf(stderr, "advciv: no SoundFont (a .sf2 in the game folder or GeneralUser-GS.sf2 next to advciv.exe):"
                            " the music is silent\n");
        live = plat_audio_open(fill, NULL) == 0;
        {   /* the music catalog, read apart from the game's catalog calls (they write file names into its data) */
            FILE *f = fopen("ACIV.CT5", "rb");
            long n = -1;
            if (f != NULL && fseek(f, 0, SEEK_END) == 0 && (n = ftell(f)) > 0 && fseek(f, 0, SEEK_SET) == 0) {
                g_ct5 = malloc((size_t)n);
                if (g_ct5 != NULL && fread(g_ct5, 1, (size_t)n, f) == (size_t)n && cat_open(&g_cat, g_ct5, (size_t)n) == 0)
                    g_cat_ok = 1;
            }
            if (f != NULL)
                fclose(f);
        }
    }
    ail_init(live ? lock : NULL, have_sf2 ? sf2 : NULL);
    if (live) {
        plat_audio_lock(1);
        g_up = 1;
        plat_audio_lock(0);
        ail_set_live(1);
    }
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

/* the GM engine plays the g set: "smap__1c.xmi" / "fmap__1c.xmi" -> "gmap__1c.xmi" (the sets share their names but
   the first letter, music.md); NULL when the catalog was not read (headless) or the file is not there */
static const uint8_t *music_image(const char *name, size_t *len)
{
    char g[16];
    const cat_entry *e;

    if (!g_cat_ok || strlen(name) >= sizeof g)
        return NULL;
    strcpy(g, name);
    if (g[0] == 'f' || g[0] == 'F' || g[0] == 's' || g[0] == 'S')
        g[0] = 'g';
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

static const snd_ops ops = {
    startup, shutdown, alloc_sample, ail_allocate_sequence,
    ail_set_sample_file, ail_set_sample_volume, ail_set_sample_loop_count, ail_start_sample, ail_end_sample,
    ail_sample_status, ail_active_sample_count,
    ail_init_sequence, ail_start_sequence, ail_stop_sequence, ail_resume_sequence, ail_end_sequence,
    ail_sequence_status, seq_volume, ail_set_sequence_loop_count, last_error, music_image, 'g'
};

void snd_ail_attach(int headless)
{
    g_headless = headless;
    snd_attach(&ops);
}
