#include "gm_synth.h"

#include <stdio.h>
#include <stdlib.h>

#include <fluidsynth.h>

struct gm_synth {
    fluid_settings_t *settings;
    fluid_synth_t *synth;
};

gm_synth *gm_synth_new(const char *sf2_path, int rate)
{
    gm_synth *g = calloc(1, sizeof *g);

    if (g == NULL)
        return NULL;
    g->settings = new_fluid_settings();
    if (g->settings == NULL)
        goto fail;
    fluid_settings_setnum(g->settings, "synth.sample-rate", (double)rate);
    fluid_settings_setstr(g->settings, "synth.midi-bank-select", "gs");  /* the g* songs use GS bank select (CC 0/32) */
    fluid_settings_setint(g->settings, "synth.cpu-cores", 1);            /* deterministic rendering */
    /* FluidSynth's default gain 0.2 peaks at 7195 of 32767 in the loudest g* song (GCIVIL2, GeneralUser GS 2026-02);
       0.6 brings that to about -3.4 dBFS. The port's mixer sets the player's music volume on top. */
    fluid_settings_setnum(g->settings, "synth.gain", 0.6);
    g->synth = new_fluid_synth(g->settings);
    if (g->synth == NULL)
        goto fail;
    if (fluid_synth_sfload(g->synth, sf2_path, 1) == FLUID_FAILED) {
        fprintf(stderr, "gm_synth: cannot load SoundFont %s\n", sf2_path);
        goto fail;
    }
    return g;
fail:
    gm_synth_free(g);
    return NULL;
}

void gm_synth_midi(void *synth, const uint8_t *msg, int len)
{
    fluid_synth_t *s = ((gm_synth *)synth)->synth;
    int ch = msg[0] & 0x0F;

    switch (msg[0] & 0xF0) {
    case 0x80: fluid_synth_noteoff(s, ch, msg[1]); break;
    case 0x90:
        if (msg[2] == 0)
            fluid_synth_noteoff(s, ch, msg[1]);
        else
            fluid_synth_noteon(s, ch, msg[1], msg[2]);
        break;
    case 0xA0: if (len == 3) fluid_synth_key_pressure(s, ch, msg[1], msg[2]); break;
    case 0xB0: if (len == 3) fluid_synth_cc(s, ch, msg[1], msg[2]); break;
    case 0xC0: fluid_synth_program_change(s, ch, msg[1]); break;
    case 0xD0: fluid_synth_channel_pressure(s, ch, msg[1]); break;
    case 0xE0: if (len == 3) fluid_synth_pitch_bend(s, ch, msg[1] | (msg[2] << 7)); break;
    default: break;
    }
}

void gm_synth_render(gm_synth *g, int16_t *out, int frames)
{
    if (frames > 0)
        fluid_synth_write_s16(g->synth, frames, out, 0, 2, out, 1, 2);
}

void gm_synth_reset(gm_synth *g)
{
    fluid_synth_system_reset(g->synth);
}

void gm_synth_free(gm_synth *g)
{
    if (g == NULL)
        return;
    if (g->synth != NULL)
        delete_fluid_synth(g->synth);
    if (g->settings != NULL)
        delete_fluid_settings(g->settings);
    free(g);
}

int gm_tick_frames(uint64_t tick, int rate)
{
    return (int)((tick + 1) * (uint64_t)rate / 120u - tick * (uint64_t)rate / 120u);
}
