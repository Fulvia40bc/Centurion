#ifndef FMDRV_H
#define FMDRV_H

/* SBPRO2.MDI (Miles AIL 3, "Creative Labs Sound Blaster Pro (new version)"), the OPL3 FM music driver, translated
   literally: the runtime of the driver from the reset to the register writes; with ADLIB.MDI ("Ad Lib Music
   Synthesizer Card"), the OPL2 driver of the f-song set, as a second mode. fmdrv.c's file comment lists the
   routines and their offsets. The caller owns time: write() is called for every OPL register write in the
   original's order, and the caller spaces them. */

#include <stddef.h>
#include <stdint.h>

typedef void (*fmdrv_write_fn)(void *user, int reg, int val);   /* reg 0x000..0x1FF: bank 1 = 0x100 | r */

typedef struct fmdrv fmdrv;

/* drv: the SBPRO2.MDI or ADLIB.MDI file image from the game folder (its tables are read from it; copied, the caller
   may free it); ADLIB.MDI selects the ADLIB mode (OPL2, 9 voices, 16 slots, mono: every write has reg 0x000..0x0FF).
   The driver's state init (drv 0x2FE1) and chip reset (0x2C47: the 492 writes; ADLIB 0x2964: 245); NULL when drv
   is neither driver as shipped or on no memory */
int    fmdrv_is_sbpro2(const uint8_t *drv, size_t len);
int    fmdrv_is_adlib(const uint8_t *drv, size_t len);
fmdrv *fmdrv_new(fmdrv_write_fn write, void *user, const uint8_t *drv, size_t len);
void   fmdrv_free(fmdrv *d);

/* the GTL file image (EHGM2FM2.OPL; EHGM2FM2.AD for ADLIB.MDI, byte-identical); 1 ok, 0 bad */
int    fmdrv_set_bank(fmdrv *d, const uint8_t *gtl, size_t len);

/* function 0x503 with an XMIDI TIMB chunk body (u16 count, then patch,bank pairs): loads each from the bank into
   the cache as the driver does; 1 ok, 0 = a timbre missing (fmdrv_failed_timbre gives bank<<8|patch) */
int    fmdrv_install_timbres(fmdrv *d, const uint8_t *timb, size_t len);
int    fmdrv_failed_timbre(const fmdrv *d);

/* one channel message as MIDI_XMIT's parser hands it to the dispatcher 0x3B54 */
void   fmdrv_midi(fmdrv *d, const uint8_t *msg, int len);

/* function 0x502: a queued buffer of messages (the 0x3ED8 parser: no running status, sysex skipped) */
void   fmdrv_xmit(fmdrv *d, const uint8_t *buf, int len);

#endif
