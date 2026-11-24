/* FM music: fmdrv.c (SBPRO2.MDI, or ADLIB.MDI) on a Nuked OPL3, as DOSBox-X plays it (fm_synth.h). Port, Claude
   23 Nov. */
#include "fm_synth.h"

#include <stdlib.h>
#include <string.h>

#include "fmdrv.h"
#include "../third_party/nuked-opl3/opl3.h"

enum { FM_NATIVE_RATE = 49716 };

/* the music's level against the digital samples, as DOSBox-X mixes them (C:\work\advciv_cmp\sound\fm\level.md):
   its OPL channel scales Nuked's int16 output by 1.5 (adlib.cpp 1702-1703, SetScale(1.5f)), the Sound Blaster's
   digital channel by 1.0, and the drivers leave the SB16 mixer at its reset values. Measured on the map song against
   DOSBox-X's own recording: 1.44..1.48 */
enum { FM_GAIN_NUM = 3, FM_GAIN_DEN = 2 };

/* DOSBox-X's "dc bias correction" (on by default, mixer.cpp 701-718) takes the slow offset off its whole mix, with a
   time constant of about 1 s; Nuked's OPL3 output sits about +110 above zero in the songs (DOSBox-X's recording: +6).
   Here a one-pole high-pass with a 1 s time constant on the music alone (the samples carry no offset) */
enum { FM_DC_SECONDS = 1 };

struct fm_synth {
    opl3_chip chip;
    fmdrv *drv;
    int buffered;           /* 0 during the install (the chip is silent: writes at once), then 1 */
    int rate;               /* the output rate */
    int swap;
    int32_t s0[2], s1[2];   /* native samples i and i + 1 around the current output sample */
    int64_t acc;            /* the output sample's position past native sample i, in 1/rate of a native sample */
    int64_t dc[2];          /* the running offset per side, in 1/65536 */
};

static void (*g_tap)(void *u, int reg, int val);   /* tests only (fm_synth_tap) */
static void *g_tap_u;

static void fm_write(void *user, int reg, int val)
{
    fm_synth *f = user;

    if (g_tap != NULL)
        g_tap(g_tap_u, reg, val);
    if (f->buffered)
        OPL3_WriteRegBuffered(&f->chip, (uint16_t)(reg & 0x1FF), (uint8_t)val);   /* DOSBox-X adlib.cpp 378 */
    else
        OPL3_WriteReg(&f->chip, (uint16_t)(reg & 0x1FF), (uint8_t)val);
}

static void fm_next(fm_synth *f)
{
    int16_t buf[2];

    f->s0[0] = f->s1[0];
    f->s0[1] = f->s1[1];
    OPL3_Generate(&f->chip, buf);       /* one native sample: the write buffer is drained at its end */
    f->s1[0] = buf[0];
    f->s1[1] = buf[1];
}

/* a GTL bank the driver can use safely (structure.md 8.2): 6-byte index records {patch, bank, u32 LE offset} up to
   one whose bank byte is 0xFF, each pointing inside the file at a timbre whose u16 size (counting itself) is 14 (2-op)
   or 25 (4-op) and fits. Port guard: the driver itself checks none of it, and a size above its 4096-byte heap would
   make 0x2F15 evict for ever under the audio lock */
static int gtl_ok(const uint8_t *g, size_t len)
{
    size_t k;

    for (k = 0; k + 2 <= len; k += 6) {
        uint32_t off, size;

        if (g[k + 1] == 0xFFu)
            return k > 0;
        if (k + 6 > len)
            return 0;
        off = (uint32_t)g[k + 2] | (uint32_t)g[k + 3] << 8 | (uint32_t)g[k + 4] << 16 | (uint32_t)g[k + 5] << 24;
        if ((uint64_t)off + 2 > len)
            return 0;
        size = (uint32_t)g[off] | (uint32_t)g[off + 1] << 8;
        if ((size != 14 && size != 25) || (uint64_t)off + size > len)
            return 0;
    }
    return 0;
}

fm_synth *fm_synth_new(const uint8_t *mdi, size_t mdi_len, const uint8_t *gtl, size_t gtl_len, int rate, int swap)
{
    fm_synth *f;

    if (mdi == NULL || gtl == NULL || rate <= 0 || (!fmdrv_is_sbpro2(mdi, mdi_len) && !fmdrv_is_adlib(mdi, mdi_len))
        || !gtl_ok(gtl, gtl_len))
        return NULL;
    f = calloc(1, sizeof *f);
    if (f == NULL)
        return NULL;
    OPL3_Reset(&f->chip, FM_NATIVE_RATE);
    f->rate = rate;
    f->swap = swap != 0;
    f->drv = fmdrv_new(fm_write, f, mdi, mdi_len);      /* 0x305 INIT_DEV: the chip reset 0x2C47, state 0x2FE1 */
    if (f->drv == NULL || !fmdrv_set_bank(f->drv, gtl, gtl_len)) {   /* 0x501 INIT_INS_MGR: the GTL file */
        fm_synth_free(f);
        return NULL;
    }
    f->buffered = 1;
    fm_next(f);
    fm_next(f);                         /* s0, s1 = native samples 0 and 1; output sample 0 sits on native 0 */
    f->acc = 0;
    return f;
}

void fm_synth_free(fm_synth *f)
{
    if (f == NULL)
        return;
    fmdrv_free(f->drv);
    free(f);
}

void fm_synth_midi(void *u, const uint8_t *msg, int len)
{
    fm_synth *f = u;

    if (f != NULL)
        fmdrv_midi(f->drv, msg, len);
}

int fm_synth_timbres(void *u, const uint8_t *timb, size_t len)
{
    fm_synth *f = u;

    if (f == NULL)
        return 0;
    return fmdrv_install_timbres(f->drv, timb, len);
}

void fm_synth_tap(void (*tap)(void *u, int reg, int val), void *u)
{
    g_tap = tap;
    g_tap_u = u;
}

int fm_synth_failed_timbre(const fm_synth *f)
{
    return f != NULL ? fmdrv_failed_timbre(f->drv) : 0;
}

static int16_t fm_out(fm_synth *f, int c, int64_t v)
{
    v = v * FM_GAIN_NUM / FM_GAIN_DEN;
    f->dc[c] += (v * 65536 - f->dc[c]) / ((int64_t)f->rate * FM_DC_SECONDS);   /* the offset, followed slowly */
    v -= f->dc[c] / 65536;
    if (v > 32767)
        v = 32767;
    if (v < -32768)
        v = -32768;
    return (int16_t)v;
}

void fm_synth_render(fm_synth *f, int16_t *out, int frames)
{
    int i, c;

    if (out == NULL || frames <= 0)
        return;
    if (f == NULL) {
        memset(out, 0, (size_t)frames * 2u * sizeof *out);
        return;
    }
    for (i = 0; i < frames; i++) {
        int16_t v[2];

        for (c = 0; c < 2; c++)         /* linear between native i and i + 1 (truncated toward zero) */
            v[c] = fm_out(f, c, f->s0[c] + (int64_t)(f->s1[c] - f->s0[c]) * f->acc / f->rate);
        out[2 * i] = f->swap ? v[1] : v[0];
        out[2 * i + 1] = f->swap ? v[0] : v[1];
        f->acc += FM_NATIVE_RATE;
        while (f->acc >= f->rate) {
            f->acc -= f->rate;
            fm_next(f);
        }
    }
}
