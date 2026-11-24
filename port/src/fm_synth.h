#ifndef FM_SYNTH_H
#define FM_SYNTH_H

/* Stereo FM music (OVERVIEW §7.2, the `s*` songs, the default set): the game's own SBPRO2.MDI, translated in fmdrv.c,
   writing a Nuked OPL3 as DOSBox-X runs it (docs: C:\work\advciv_cmp\sound\fm\dosbox_opl.md):
   - the chip runs at its native 49,716 Hz; every register write after the chip reset goes through Nuked's own write
     buffer (OPL3_WriteRegBuffered, as DOSBox-X's NukedOPL handler does), so two writes land at least 2 chip samples
     apart and a key-off right before a key-on still retriggers the note;
   - the output is resampled to the caller's rate by linear interpolation (DOSBox-X's mixer does the same to its own
     rate), in integer maths, so ARM64 and x64 give the same samples;
   - the sides are the chip's: SBPRO2 sends MIDI-left to the second output, played on the right (Erik's ruling of
     23 Nov: as heard in DOSBox-X); swap = 1 mirrors them.
   The caller drives time as for gm_synth: the channel messages of a 120 Hz tick, then that tick's frames.
   Nuked-OPL3 is LGPL-2.1 (port/third_party/nuked-opl3/LICENSE). */

#include <stddef.h>
#include <stdint.h>

typedef struct fm_synth fm_synth;

/* mdi: SBPRO2.MDI, gtl: EHGM2FM2.OPL, both file images from the game folder (copied; the caller may free them).
   The driver's install (0x305: the 492-write chip reset, then 0x501: the bank). NULL when the driver is not the
   shipped SBPRO2.MDI, the bank is malformed (an index entry or timbre outside the file, a timbre size other than 14
   or 25), or on no memory. A well-formed bank without one of a song's timbres fails that song's 0x503, as in the
   original. */
fm_synth *fm_synth_new(const uint8_t *mdi, size_t mdi_len, const uint8_t *gtl, size_t gtl_len, int rate, int swap);
void      fm_synth_free(fm_synth *f);

void      fm_synth_midi(void *f, const uint8_t *msg, int len);          /* an xmi_out_fn: one channel message */
/* AIL_init_sequence's 0x503 with an XMIDI TIMB chunk body (u16 LE count, count x {patch, bank}); 1 ok, 0 a timbre
   is missing: fm_synth_failed_timbre gives bank << 8 | patch */
int       fm_synth_timbres(void *f, const uint8_t *timb, size_t len);
int       fm_synth_failed_timbre(const fm_synth *f);
void      fm_synth_render(fm_synth *f, int16_t *out, int frames);        /* interleaved stereo at the caller's rate */

/* tests: every register write of every fm_synth, its install's too, also goes to tap (reg 0x000..0x1FF), in the
   driver's order; NULL stops it */
void      fm_synth_tap(void (*tap)(void *u, int reg, int val), void *u);

#endif
