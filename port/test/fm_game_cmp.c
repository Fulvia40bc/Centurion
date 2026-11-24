/* fm_game_cmp GAMEDIR: the FM music as the game plays it, against the original's own host model, for both FM sets:
   the stereo FM set (S*) through SBPRO2.MDI and the FM set (F*) through ADLIB.MDI (whose own capture, FMAP__1C,
   fm_dro_cmp matches 5,905 of 5,905).
   Side B is the port's game path: ail.c's facade (ail_init_fm, ail_init_sequence, ail_set_sequence_volume,
   ail_start_sequence, ail_set_sequence_loop_count, ail_stop/resume/end_sequence) on musplay.c, xmiseq.c and fm_synth.c
   (fmdrv.c on Nuked), driven by ail_render one 120 Hz tick of output at a time, with every OPL register write caught
   by fm_synth_tap. Side A is the original's AIL host as fm_dro_cmp.c models it: the driver install (fmdrv_new,
   fmdrv_set_bank), the channel set-up (16 x 15 messages), then per song f7B854's calls: the stop of the old song,
   0x503 with the song's raw TIMB chunk, and the XMIDI interpreter xmi_host.c (verified message for message against
   fm_dro_cmp.c's model, whose writes match the DOSBox-X capture of SMAP__1C 9,149 of 9,149), each message to its own
   fmdrv instance. A and B must write the same registers with the same values in the same order, and in the same
   120 Hz tick (printed per segment as the range of A-B tick differences: 0..0). The port starts a song at once
   (mus_start's advance(0)) where the host's first interval comes at the next timer tick, up to 8.3 ms later; side
   A runs that interval at the start too, so the music toggle meets both at the same point of the song.
   The script, per driver: its four songs in turn (MAP__1C, CIVIL2, TRADE2C, CALAM5J), each played for a while, with
   the music toggle (stop, a second stop, resume) in each, at the game's volumes 9 and 7 (127 and 98). Exit 0 when all
   writes agree, else 1 (2 on a file error). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/ail.h"
#include "../src/catalog.h"
#include "../src/fm_synth.h"
#include "../src/fmdrv.h"
#include "xmi_host.h"

typedef struct { uint16_t reg; uint8_t val; int32_t t; } wr;
typedef struct { wr *v; size_t n, cap; int32_t stamp; } wlog;

static void wlog_add(wlog *l, int reg, int val)
{
    if (l->n == l->cap) {
        l->cap = l->cap ? l->cap * 2 : 1 << 16;
        l->v = realloc(l->v, l->cap * sizeof *l->v);
        if (l->v == NULL) {
            fprintf(stderr, "fm_game_cmp: no memory\n");
            exit(2);
        }
    }
    l->v[l->n].reg = (uint16_t)(reg & 0x1FF);
    l->v[l->n].val = (uint8_t)val;
    l->v[l->n].t = l->stamp;
    l->n++;
}

static void tap_b(void *u, int reg, int val) { wlog_add(u, reg, val); }
static void on_write_a(void *u, int reg, int val) { wlog_add(u, reg, val); }

static uint8_t *load(const char *dir, const char *name, size_t *n)
{
    char path[1024];
    FILE *f;
    uint8_t *b;
    long k;

    snprintf(path, sizeof path, "%s/%s", dir, name);
    f = fopen(path, "rb");
    if (f == NULL)
        return NULL;
    fseek(f, 0, SEEK_END);
    k = ftell(f);
    fseek(f, 0, SEEK_SET);
    b = malloc(k > 0 ? (size_t)k : 1);
    if (b == NULL || fread(b, 1, (size_t)k, f) != (size_t)k) {
        fclose(f);
        free(b);
        return NULL;
    }
    fclose(f);
    *n = (size_t)k;
    return b;
}

/* the body of the first chunk named id in an XMI image (a plain scan, independent of xmi.c) */
static int chunk(const uint8_t *d, size_t n, const char *id, const uint8_t **body, uint32_t *len)
{
    size_t i;

    for (i = 0; i + 8 <= n; i++)
        if (memcmp(d + i, id, 4) == 0) {
            uint32_t l = ((uint32_t)d[i + 4] << 24) | ((uint32_t)d[i + 5] << 16) | ((uint32_t)d[i + 6] << 8) | d[i + 7];
            if (i + 8 + l > n)
                return 0;
            *body = d + i + 8;
            *len = l;
            return 1;
        }
    return 0;
}

/* side A: the host model */
typedef struct {
    fmdrv *drv;
    xmi_host *h;              /* the current song's interpreter, NULL before the first */
    size_t sent;              /* records of h already given to the driver */
} side_a;

static void a_drain(side_a *a)
{
    size_t i;

    if (a->h == NULL)
        return;
    for (i = a->sent; i < xmi_host_count(a->h); i++) {
        const xmi_host_rec *r = &xmi_host_recs(a->h)[i];
        fmdrv_midi(a->drv, r->msg, r->len);
    }
    a->sent = xmi_host_count(a->h);
}

static int fails;

/* compare the writes of one segment: same registers, values and order; the tick differences printed */
static void compare(const char *what, const wlog *a, size_t a0, const wlog *b, size_t b0)
{
    size_t na = a->n - a0, nb = b->n - b0, i, n = na < nb ? na : nb;
    int32_t dmin = 0, dmax = 0;

    for (i = 0; i < n; i++) {
        const wr *x = &a->v[a0 + i], *y = &b->v[b0 + i];
        int32_t d = x->t - y->t;

        if (x->reg != y->reg || x->val != y->val) {
            printf("  FAIL %s: write %zu of the segment: host %03X=%02X (tick %d), game %03X=%02X (tick %d)\n", what, i,
                   x->reg, x->val, (int)x->t, y->reg, y->val, (int)y->t);
            fails++;
            return;
        }
        if (i == 0 || d < dmin)
            dmin = d;
        if (i == 0 || d > dmax)
            dmax = d;
    }
    if (na != nb) {
        printf("  FAIL %s: host %zu writes, game %zu\n", what, na, nb);
        fails++;
        return;
    }
    if (dmin != 0 || dmax != 0) {
        printf("  FAIL %s: the writes agree but not their 120 Hz ticks (host - game %d..%d)\n", what, (int)dmin,
               (int)dmax);
        fails++;
        return;
    }
    printf("  %-34s %7zu writes equal, tick host - game %d..%d\n", what, na, (int)dmin, (int)dmax);
}

int main(int argc, char **argv)
{
    static const char *tails[4] = { "MAP__1C.XMI", "CIVIL2.XMI", "TRADE2C.XMI", "CALAM5J.XMI" };
    static const char *drvs[2] = { "SBPRO2.MDI", "ADLIB.MDI" }, *banks[2] = { "EHGM2FM2.OPL", "EHGM2FM2.AD" };
    static const char letters[2] = { 'S', 'F' };  /* the stereo FM set through SBPRO2, the FM set through ADLIB */
    size_t ct5n = 0, mdins[2] = { 0, 0 }, gtlns[2] = { 0, 0 }, mdin, gtln;
    uint8_t *ct5, *mdis[2], *gtls[2], *mdi, *gtl;
    catalog cat;
    wlog la, lb;
    side_a a;
    int32_t T = 0;            /* host ticks processed so far */
    int32_t seqh;
    int s, vol, cfg, d;

    if (argc < 2) {
        fprintf(stderr, "usage: fm_game_cmp GAMEDIR\n");
        return 2;
    }
    ct5 = load(argv[1], "ACIV.CT5", &ct5n);
    for (d = 0; d < 2; d++) {
        mdis[d] = load(argv[1], drvs[d], &mdins[d]);
        gtls[d] = load(argv[1], banks[d], &gtlns[d]);
        if (mdis[d] == NULL || gtls[d] == NULL) {
            fprintf(stderr, "fm_game_cmp: cannot read %s or %s in %s\n", drvs[d], banks[d], argv[1]);
            return 2;
        }
    }
    if (ct5 == NULL || cat_open(&cat, ct5, ct5n) != 0) {
        fprintf(stderr, "fm_game_cmp: cannot read ACIV.CT5 in %s\n", argv[1]);
        return 2;
    }
    memset(&la, 0, sizeof la);
    memset(&lb, 0, sizeof lb);
    memset(&a, 0, sizeof a);

    for (cfg = 0; cfg < 4; cfg++) {                  /* each driver at music volumes 9 and 7 */
        int seqvol;
        size_t a0 = la.n, b0 = lb.n;

        d = cfg / 2;
        vol = cfg % 2 ? 7 : 9;
        seqvol = vol * 127 / 9;                      /* f7B854 0x7B9FB..0x7BA27 */
        mdi = mdis[d];
        mdin = mdins[d];
        gtl = gtls[d];
        gtln = gtlns[d];
        printf("%s, music volume %d (%d):\n", drvs[d], vol, seqvol);
        /* the install: A as fm_dro_cmp's main, B as snd_ail.c's start_fm */
        la.stamp = lb.stamp = T = -1;
        a.drv = fmdrv_new(on_write_a, &la, mdi, mdin);
        if (a.drv == NULL || !fmdrv_set_bank(a.drv, gtl, gtln)) {
            fprintf(stderr, "fm_game_cmp: the host model's driver did not install\n");
            return 2;
        }
        {   /* the channel set-up (0x9AD90..0x9AE96, fm_dro_cmp.c host_channel_setup) */
            static const uint8_t ccs[12][2] = { { 112, 0 }, { 1, 0 }, { 7, 127 }, { 10, 64 }, { 11, 127 }, { 64, 0 },
                                                { 91, 40 }, { 93, 0 }, { 100, 0 }, { 101, 0 }, { 38, 0 }, { 6, 2 } };
            int ch, i;
            for (ch = 0; ch < 16; ch++) {
                uint8_t m[3];
                m[0] = (uint8_t)(0xB0 | ch); m[1] = 114; m[2] = 0; fmdrv_midi(a.drv, m, 3);
                m[0] = (uint8_t)(0xC0 | ch); m[1] = 0; fmdrv_midi(a.drv, m, 2);
                m[0] = (uint8_t)(0xE0 | ch); m[1] = 0; m[2] = 0x40; fmdrv_midi(a.drv, m, 3);
                for (i = 0; i < 12; i++) {
                    m[0] = (uint8_t)(0xB0 | ch); m[1] = ccs[i][0]; m[2] = ccs[i][1]; fmdrv_midi(a.drv, m, 3);
                }
            }
        }
        fm_synth_tap(tap_b, &lb);
        if (!ail_init_fm(NULL, mdi, mdin, gtl, gtln, 0)) {
            fprintf(stderr, "fm_game_cmp: ail_init_fm failed\n");
            return 2;
        }
        seqh = ail_allocate_sequence();
        compare("install and channel set-up", &la, a0, &lb, b0);
        T = 0;

        for (s = 0; s < 4; s++) {
            char song[16];
            const cat_entry *e;
            const uint8_t *x, *evnt, *timb;
            uint32_t elen, tlen;
            int32_t k, play = 120 * (s == 0 ? 95 : 41);
            char what[64];

            snprintf(song, sizeof song, "%c%s", letters[d], tails[s]);
            e = cat_find(&cat, song);
            x = e != NULL ? cat_data(&cat, e) : NULL;
            a0 = la.n;
            b0 = lb.n;
            if (x == NULL || !chunk(x, e->length, "EVNT", &evnt, &elen) || !chunk(x, e->length, "TIMB", &timb, &tlen)) {
                fprintf(stderr, "fm_game_cmp: %s not usable\n", song);
                return 2;
            }
            la.stamp = lb.stamp = T;
            /* f7B854: AIL_end_sequence of the old song (its stop, gated on playing), AIL_init_sequence (0x503 with
               the TIMB chunk), the volume, AIL_start_sequence, AIL_set_sequence_loop_count(0) */
            if (a.h != NULL) {
                xmi_host_stop(a.h);
                a_drain(&a);
                xmi_host_free(a.h);
            }
            if (!fmdrv_install_timbres(a.drv, timb, tlen)) {
                fprintf(stderr, "fm_game_cmp: %s: the host model's 0x503 failed\n", song);
                return 2;
            }
            a.h = xmi_host_new(evnt, elen);
            a.sent = 0;
            xmi_host_set_volume(a.h, seqvol);
            xmi_host_reset(a.h);
            xmi_host_start(a.h);
            xmi_host_set_loops(a.h, 0);
            xmi_host_tick(a.h);                      /* the song's first interval now: the port starts a song at */
            a_drain(&a);                             /* once (mus_start's advance(0)), so both sides stay in step */
            ail_end_sequence(seqh);
            if (!ail_init_sequence(seqh, x, e->length, 0)) {
                printf("  FAIL %s: ail_init_sequence: %s", song, ail_last_error());
                fails++;
                break;
            }
            ail_set_sequence_volume(seqh, seqvol, 0);
            ail_start_sequence(seqh);
            ail_set_sequence_loop_count(seqh, 0);
            /* play, with the music toggle (menu2.c: AIL_stop_sequence, AIL_resume_sequence) at fixed points: a stop,
               a second stop while stopped (sends nothing), 2 s of silence, the resume (the 0x99C20 replay) */
            for (k = 0; k < play; k++) {
                static int16_t buf[2 * 2048];
                int frames = (int)((int64_t)(T + 1) * 44100 / 120 - (int64_t)T * 44100 / 120);

                la.stamp = T + 1;                    /* host interval T + 1 */
                xmi_host_tick(a.h);
                a_drain(&a);
                lb.stamp = T + 1;                    /* the game side's writes of this tick come at its end */
                ail_render(buf, frames);
                T++;
                la.stamp = lb.stamp = T;
                if (k == play / 3 || k == play / 3 + 7) {
                    xmi_host_stop(a.h);
                    a_drain(&a);
                    ail_stop_sequence(seqh);
                }
                if (k == play / 3 + 240) {
                    xmi_host_resume(a.h);
                    a_drain(&a);
                    ail_resume_sequence(seqh);
                }
            }
            snprintf(what, sizeof what, "%s (%d s, toggle)", song, (int)(play / 120));
            compare(what, &la, a0, &lb, b0);
        }
        la.stamp = lb.stamp = T;
        a0 = la.n;
        b0 = lb.n;
        xmi_host_stop(a.h);                          /* AIL_end_sequence at the quit */
        a_drain(&a);
        xmi_host_free(a.h);
        a.h = NULL;
        ail_end_sequence(seqh);
        compare("the end", &la, a0, &lb, b0);
        fm_synth_tap(NULL, NULL);
        ail_shutdown();
        fmdrv_free(a.drv);
        a.drv = NULL;
    }
    printf("%s\n", fails ? "fm_game_cmp: DIFFERENCES" : "fm_game_cmp: all writes equal");
    free(la.v);
    free(lb.v);
    free(ct5);
    for (d = 0; d < 2; d++) {
        free(mdis[d]);
        free(gtls[d]);
    }
    return fails ? 1 : 0;
}
