/* plat_demo GAMEDIR [--headless OUT.png] [--sf2 FILE] [--song NAME.XMI]
   M1 check: the title pictures P2, P3, P4 (640x480 GIF87a) through platform.h, with a General MIDI song from ACIV.CT5
   playing (xmiseq + gm_synth). Click or Space: next picture; Esc or closing the window: quit; Alt+Enter / F11: full
   screen. Keys and clicks are echoed on stderr (start it from a terminal to see them). --headless opens nothing and
   saves P2 as an indexed PNG. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/catalog.h"
#include "../src/gif.h"
#include "../src/gm_synth.h"
#include "../src/palette.h"
#include "../src/platform.h"
#include "../src/xmi.h"
#include "../src/xmiseq.h"

static uint8_t *load_file(const char *path, size_t *size)
{
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    long n;

    if (f == NULL)
        return NULL;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = malloc(n > 0 ? (size_t)n : 1);
    if (buf == NULL || fread(buf, 1, (size_t)n, f) != (size_t)n) {
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *size = (size_t)n;
    return buf;
}

static uint8_t fb[SCREEN_W * SCREEN_H];
static uint8_t pal[256][3];

static int show_gif(const char *dir, const char *name)
{
    char path[1024];
    size_t size;
    uint8_t *data;
    gif_image img;
    int x, y, rc;

    snprintf(path, sizeof path, "%s/%s", dir, name);
    data = load_file(path, &size);
    if (data == NULL) {
        fprintf(stderr, "plat_demo: cannot read %s\n", path);
        return -1;
    }
    rc = gif_decode(data, size, &img);
    free(data);
    if (rc != 0) {
        fprintf(stderr, "plat_demo: %s: gif_decode %d\n", path, rc);
        return -1;
    }
    memset(fb, 0, sizeof fb);
    {   /* through the 6-bit DAC, as the original shows it (palette.h) */
        uint8_t dac[256][3];
        pal_gif_to_dac((const uint8_t (*)[3])img.pal, img.ncolors, dac);
        pal_dac_to_rgb((const uint8_t (*)[3])dac, pal);
    }
    for (y = 0; y < img.h && img.top + y < SCREEN_H; y++)
        for (x = 0; x < img.w && img.left + x < SCREEN_W; x++)
            fb[(img.top + y) * SCREEN_W + img.left + x] = img.pixels[(size_t)y * img.w + x];
    gif_free(&img);
    plat_present(fb, (const uint8_t (*)[3])pal);
    return 0;
}

/* the music, run on the audio thread: one sequencer tick, then that tick's samples */
typedef struct { xmi_seq *seq; gm_synth *g; uint64_t tick; int left; } music;

static void fill(void *user, int16_t *out, int frames)
{
    music *m = user;

    while (frames > 0) {
        int n;
        if (m->left == 0) {
            xmi_seq_advance(m->seq, m->tick == 0 ? 0 : 1);
            m->left = gm_tick_frames(m->tick++, PLAT_RATE);
        }
        n = m->left < frames ? m->left : frames;
        gm_synth_render(m->g, out, n);
        out += n * 2;
        frames -= n;
        m->left -= n;
    }
}

int main(int argc, char **argv)
{
    static const char *pics[] = { "P2", "P3", "P4" };
    const char *dir, *png = NULL, *sf2 = "C:/work/toolchain/soundfont/GeneralUser-GS.sf2", *song = "GCIVIL2.XMI";
    char path[1024];
    uint8_t *ct5 = NULL;
    size_t size;
    catalog cat;
    const cat_entry *e;
    xmi_song xs;
    music m;
    int i, pic = 0, quit = 0, have_music = 0;

    plat_console();
    if (argc < 2) {
        fprintf(stderr, "usage: plat_demo GAMEDIR [--headless OUT.png] [--sf2 FILE] [--song NAME.XMI]\n");
        return 2;
    }
    dir = argv[1];
    for (i = 2; i + 1 < argc; i += 2) {
        if (strcmp(argv[i], "--headless") == 0) png = argv[i + 1];
        else if (strcmp(argv[i], "--sf2") == 0) sf2 = argv[i + 1];
        else if (strcmp(argv[i], "--song") == 0) song = argv[i + 1];
        else { fprintf(stderr, "plat_demo: unknown option %s\n", argv[i]); return 2; }
    }
    if (plat_init("Advanced Civilization - port demo", png != NULL ? PLAT_HEADLESS : 0) != 0)
        return 1;
    if (show_gif(dir, pics[0]) != 0)
        return 1;
    if (png != NULL) {
        int rc = plat_save_png(png);
        plat_shutdown();
        if (rc != 0)
            fprintf(stderr, "plat_demo: cannot write %s\n", png);
        return rc != 0;
    }

    memset(&m, 0, sizeof m);
    snprintf(path, sizeof path, "%s/ACIV.CT5", dir);
    ct5 = load_file(path, &size);
    if (ct5 != NULL && cat_open(&cat, ct5, size) == 0 && (e = cat_find(&cat, song)) != NULL &&
        xmi_parse(cat_data(&cat, e), e->length, &xs) == 0 && (m.g = gm_synth_new(sf2, PLAT_RATE)) != NULL &&
        (m.seq = xmi_seq_new(&xs, gm_synth_midi, NULL, m.g)) != NULL)
        have_music = plat_audio_open(fill, &m) == 0;
    if (!have_music)
        fprintf(stderr, "plat_demo: no music (%s, %s)\n", song, sf2);

    while (!quit) {
        plat_event ev;
        while (plat_poll(&ev)) {
            if (ev.type == PLAT_QUIT) {
                quit = 1;
            } else if (ev.type == PLAT_KEY) {
                fprintf(stderr, "key scan %02X ascii %02X mods %d\n", ev.scan, ev.ascii, ev.mods);
                if (ev.ascii == 27)
                    quit = 1;
                else if (ev.ascii == ' ')
                    show_gif(dir, pics[pic = (pic + 1) % 3]);
            } else if (ev.type == PLAT_MOUSE_DOWN) {
                fprintf(stderr, "button %d down at %d,%d\n", ev.button, ev.x, ev.y);
                if (ev.button == PLAT_BUTTON_LEFT)
                    show_gif(dir, pics[pic = (pic + 1) % 3]);
            }
        }
        plat_sleep_ms(10);
    }
    plat_shutdown();   /* closes the audio stream first: the callback is done before the music goes */
    if (have_music) {
        xmi_seq_free(m.seq);
        gm_synth_free(m.g);
        xmi_free(&xs);
    }
    free(ct5);
    return 0;
}
