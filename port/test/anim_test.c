/* anim_test [GAMEDIR [CATDIR]]: unit checks for module ANIM (anim.c: 0x7A0A0, 0x79ADC, 0x79C70, 0x7A4A0, 0x7A4C4) that
   need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read); CATDIR holds
   ACIV.CT1 and ACIV.CT4 (default C:\work\advciv_play, only read). The catalogs are opened with fg.c's cat_open_file
   as the start-up does ([0xF40D4], [0xF40E0]). The test runs in a scratch folder of its own (the current directory
   when it starts must be a scratch folder: the port writes "aciv.cfg" relative to it, as the original does); it
   refuses to run where an ACIV.CFG or AC_MAIN.EXE lies, so the real configuration is never touched.

   fg.c runs headless (fg_set_headless(1)); fg_set_input_hook captures every frame fg_showflic shows (a hash of the
   screen and the DAC each time it polls the keyboard). The routines anim.c calls that are not under test are stubs
   that append to a trace: menu2.c's f7B504 (slot, name), ui_screen.c's f78884 / f788AC ("H" / "S"), f8240F (the
   buffer and size it is given); game_rand f78AE4 is the LCG of game.h on the seed [0xCAC5C], counted. fg.c, gif.c,
   junk.c ("junk2" in memory), load.c's heap and the catalogs are the real ones.

   The model is a second implementation written from the listing (0x7A0A0..0x7A49E, 0x79ADC..0x79C6C,
   0x79C70..0x79E61), not from anim.c: the counter as a 6-bit field, the roll as r * 10 >> 15, the WAV / FLC names
   from the pointer tables, the catalogs parsed here from the files' bytes (not through cat_*), the FLC header's
   width / height read here, the picture centred in the map window, and the expected frames produced by playing the
   FLC bytes extracted here through fg_showflic at the model's position on the same starting screen and palette. For
   each case the port's and the model's outcomes must be equal: the game_rand count, the f7B504 trace, the mouse
   trace, the f8240F calls, aciv.cfg (absent, or its 6 bytes), the whole data object (snapshot plus the model's
   changes), the frame hashes, the final screen and DAC, and (0x79C70) the back buffer. Parts:
     A. 0x7A0A0 for every calamity 0x12..0x1E on a grid of states: counter 0 with roll 0, counter 0 with a roll 1..9,
        counter 1, 2, 51, 63; animations on / off; digital sound on / off; effects on / off; "already shown" (same id
        as [0xCAC64]); the other bits of aciv.cfg bytes 4..5 random. Plus hand checks: kaboom3's file bytes, the
        counter's decrement path keeping bits 8 and 15, the position (210, 90) of every FLC.
     B. random states (500): id, counter, roll, flags, the bits around the counter, [0xCAC64].
     C. 0x7A4C4 (animations on: the current calamity [0xF783C]; off: nothing, not even the roll) and 0x7A4A0 (the next
        call plays again).
     D. 0x79ADC (zeus.flc, zeus.wav) and 0x79C70 (brpc.gif with "": the picture on the screen at (160, 65), the back
        buffer restored, the palette kept).
     E. mutation check: deliberately broken models (one change each) must disagree with the port on parts A and B.
   Prints each failure, the counts per part, and exits 0 when all pass.

   Build (from a scratch folder; R = the repo's port folder):
     clang -std=c11 -Wall -Wextra -Werror -pedantic -IR/src R/test/anim_test.c R/src/anim.c R/src/fg.c R/src/gif.c
       R/src/catalog.c R/src/fgfont.c R/src/palette.c R/src/png_write.c R/src/flc.c R/src/junk.c R/src/load.c
       R/src/state.c R/src/le_image.c -o anim_test.exe */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "anim.h"
#include "fg.h"
#include "game.h"
#include "gif.h"
#include "platform.h"
#include "state.h"

/* ---- stubs ------------------------------------------------------------------------------------------------------- */

static uint64_t s_ticks;
uint64_t plat_ticks_ms(void) { s_ticks += 16; return s_ticks; }
void plat_sleep_ms(uint32_t ms) { s_ticks += ms; }
int plat_pointer(int on) { return on; }
void plat_present(const uint8_t *fb, const uint8_t pal[256][3]) { (void)fb; (void)pal; }
int plat_poll(plat_event *ev) { (void)ev; return 0; }
void plat_mouse(int *x, int *y, int *buttons) { *x = 0; *y = 0; *buttons = 0; }
void plat_shutdown(void) {}

static char g_snd[512];      /* f7B504 trace */
static char g_mouse[64];     /* f78884 / f788AC trace */
static char g_ibuf[256];     /* f8240F calls */
static int g_rand;           /* game_rand calls */

static void app(char *dst, size_t n, const char *s)
{
    size_t l = strlen(dst);
    if (l + strlen(s) + 1 < n)
        strcat(dst, s);
}

uint8_t *ui_heap_F522C;
void f7B504(int s, const char *name)
{
    char b[96];
    snprintf(b, sizeof b, "(%d,%s)", s, name);
    app(g_snd, sizeof g_snd, b);
}
void f78884(void) { app(g_mouse, sizeof g_mouse, "H"); }
void f788AC(void) { app(g_mouse, sizeof g_mouse, "S"); }
int f8240F(uint8_t *buffer, uint32_t size)
{
    char b[64];
    snprintf(b, sizeof b, "(%s,%X)", buffer == ui_heap_F522C ? "F522C" : buffer == NULL ? "null" : "blk", (unsigned)size);
    app(g_ibuf, sizeof g_ibuf, b);
    return 0;
}
int f78AE4(void)
{
    uint32_t s = D32(0xCAC5Cu) * 0x15A4E35u + 1u;
    setD32(0xCAC5Cu, s);
    g_rand++;
    return (int)((s >> 16) & 0x7FFF);
}
_Noreturn void not_ported(uint32_t addr)
{
    printf("FAIL not_ported(0x%X)\n", (unsigned)addr);
    exit(2);
}
void (*g_not_ported_hook)(uint32_t addr);
_Noreturn void f44CAC(void) { printf("FAIL quit: %s\n", DSTR(0xF5500u)); exit(2); }
void f3A780(const char *msg) { printf("FAIL 0x3A780: %s\n", msg); exit(2); }   /* load.c's file loaders, not called */

/* ---- checks ------------------------------------------------------------------------------------------------------ */

static int g_fails, g_quiet;
#define CHECK(cond, ...) do { if (!(cond)) { g_fails++; if (!g_quiet) { printf("FAIL "); printf(__VA_ARGS__); \
                              printf(" (line %d)\n", __LINE__); } } } while (0)

static uint64_t fnv(const uint8_t *p, size_t n, uint64_t h)
{
    size_t i;
    for (i = 0; i < n; i++)
        h = (h ^ p[i]) * 0x100000001B3ull;
    return h;
}
static uint64_t screen_hash(void)
{
    return fnv((const uint8_t *)fg_dac(), 768, fnv(fg_screen(), 640u * 480u, 0xCBF29CE484222325ull));
}

/* frame capture through fg's input hook */
#define MAXF 256
static int g_cap, g_nf;
static uint64_t g_fr[MAXF];
static void hook(void)
{
    if (g_cap && g_nf < MAXF)
        g_fr[g_nf++] = screen_hash();
}

/* ---- the catalogs, parsed here (24-byte entries, 32 per block, then a link entry of 12 x 0xFF) --------------------- */

typedef struct { uint8_t *d; size_t n; } file_t;
static file_t g_ct1, g_ct4;

static file_t read_all(const char *path)
{
    file_t f = { NULL, 0 };
    FILE *fp = fopen(path, "rb");
    if (fp == NULL)
        return f;
    fseek(fp, 0, SEEK_END);
    f.n = (size_t)ftell(fp);
    fseek(fp, 0, SEEK_SET);
    f.d = malloc(f.n);
    if (f.d == NULL || fread(f.d, 1, f.n, fp) != f.n) {
        free(f.d);
        f.d = NULL;
        f.n = 0;
    }
    fclose(fp);
    return f;
}
static uint32_t le32(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }
static int le16s(const uint8_t *p) { return (int16_t)(p[0] | p[1] << 8); }

/* the entry's bytes (case-insensitive name), or NULL */
static const uint8_t *entry(const file_t *f, const char *name, uint32_t *len)
{
    uint32_t base = 0;
    int guard = 0;
    while (guard++ < 64) {
        uint32_t link = 0;
        int i, j;
        for (i = 0; i < 33; i++) {
            const uint8_t *e = f->d + base + 24u * (uint32_t)i;
            int ff = 1, eq = 1;
            for (j = 0; j < 12; j++)
                if (e[j] != 0xFF)
                    ff = 0;
            if (ff) {
                link = le32(e + 12);
                break;
            }
            for (j = 0; j < 12; j++) {
                int a = e[j], b = name[j];
                if (b >= 'a' && b <= 'z')
                    b -= 32;
                if (a != b) {
                    eq = 0;
                    break;
                }
                if (a == 0)
                    break;
            }
            if (eq && e[0] != 0) {
                *len = le32(e + 16);
                return f->d + le32(e + 12);
            }
        }
        if (link == 0)
            return NULL;
        base = link;
    }
    return NULL;
}

/* ---- the starting screen ----------------------------------------------------------------------------------------- */

static uint8_t g_scr0[640 * 480], g_dac0[768];
static uint8_t g_bb[640 * 480], g_bb0[640 * 480];   /* the back buffer's memory and its starting content */

static void start_screen(void)
{
    memcpy(fg_screen(), g_scr0, sizeof g_scr0);
    fg_setdacs(0, 256, g_dac0);
    memcpy(g_bb, g_bb0, sizeof g_bb);
    fg_move(3, 4);
}

/* ---- names from the pointer tables (the model reads them itself) -------------------------------------------------- */

static const char *tab(uint32_t base, int id) { return DSTR(D32(base + 4u * (uint32_t)id)); }

/* ---- the model ---------------------------------------------------------------------------------------------------- */

enum { M_NONE, M_SET50, M_ROLL14, M_NOWRITE_DEC, M_CENTRE, M_ALL_ALT, M_RAND_ALWAYS, M_MASK80, M_NO_SHOWN, M_NO_RELOAD,
       M_COUNT };
static const char *const g_mut_name[M_COUNT] = { "none", "counter set to 50", "roll >> 14",
    "no aciv.cfg write on the count-down", "x one to the right", "every calamity has kaboom3", "roll even while the counter runs",
    "bit 8 not kept (and 0x80)", "no already-shown test", "no reload of the normal sound" };

typedef struct {
    int rand;
    char snd[512], mouse[64], ibuf[256];
    int cfg_written;
    uint8_t cfg[6];
    uint8_t *ds;             /* the whole data object afterwards (the live image; NULL once the outcome is kept) */
    uint64_t dsh;            /* its hash */
    int nf;
    uint64_t fr[MAXF];
    uint64_t final;          /* screen + DAC afterwards */
} outcome;

/* model frames for one FLC at (x, y) from the starting screen, cached */
typedef struct { const uint8_t *flc; int x, y, nf; uint64_t fr[MAXF]; uint64_t final; } fcache;
static fcache g_fc[64];
static int g_nfc;
static uint8_t *g_play;      /* a copy of the FLC bytes for fg_imagebuf */

static const fcache *model_frames(const uint8_t *flc, uint32_t len, int x, int y)
{
    int i;
    fcache *c;
    for (i = 0; i < g_nfc; i++)
        if (g_fc[i].flc == flc && g_fc[i].x == x && g_fc[i].y == y)
            return &g_fc[i];
    c = &g_fc[g_nfc < 64 ? g_nfc++ : 63];
    c->flc = flc;
    c->x = x;
    c->y = y;
    start_screen();
    memcpy(g_play, flc, len);
    fg_imagebuf(g_play, len);
    fg_move(x, y);
    g_cap = 1;
    g_nf = 0;
    fg_showflic("", 1, 6);
    g_cap = 0;
    c->nf = g_nf;
    memcpy(c->fr, g_fr, sizeof g_fr);
    memcpy(fg_screen(), g_scr0, sizeof g_scr0);   /* the area put back: the whole screen as it was, the DAC as left */
    c->final = screen_hash();
    fg_imagebuf(NULL, 0);
    return c;
}

/* the model of 0x7A0A0 on the data object image m (a copy), starting screen, the files parsed here */
static void model_7A0A0(int mut, uint8_t *m, int id, outcome *o)
{
#define MB(a) m[(a) - DS_BASE]
#define MW(a) ((uint32_t)MB(a) | (uint32_t)MB((a) + 1) << 8)
#define MD(a) ((uint32_t)MB(a) | (uint32_t)MB((a) + 1) << 8 | (uint32_t)MB((a) + 2) << 16 | (uint32_t)MB((a) + 3) << 24)
    uint32_t last = MD(0xCAC64u), word, cnt, seed;
    int anim = (MB(0xF097Cu) & 4) != 0, digital = MB(0xF625Cu) != 0, alt_on = 0, i, r, x = 0, y = 0;
    const char *wav, *alt;
    char b[96];
    const uint8_t *flc = NULL;
    uint32_t flen = 0;

    memset(o, 0, sizeof *o);
    o->final = 0;
    if (mut != M_NO_SHOWN && last == (uint32_t)id) {
        start_screen();
        o->final = screen_hash();
        o->ds = m;
        o->dsh = fnv(m, DS_SIZE, 1);
        return;
    }
    for (i = 0; i < 4; i++) {
        MB(0xCAC64u + (uint32_t)i) = (uint8_t)((uint32_t)id >> (8 * i));
        MB(0xCAC60u + (uint32_t)i) = (uint8_t)((uint32_t)id >> (8 * i));
    }
    wav = tab(0xCAA38u, id);
    alt = tab(0xCAA6Cu, id);
    if (mut == M_ALL_ALT && alt[0] == 0)
        alt = "kaboom3.wav";
    word = MW(0xF0980u);
    cnt = (word >> 9) & 0x3F;
    if (cnt == 0 || mut == M_RAND_ALWAYS) {
        seed = MD(0xCAC5Cu) * 0x15A4E35u + 1u;
        for (i = 0; i < 4; i++)
            MB(0xCAC5Cu + (uint32_t)i) = (uint8_t)(seed >> (8 * i));
        o->rand++;
        r = (int)((((seed >> 16) & 0x7FFF) * 10u) >> (mut == M_ROLL14 ? 14 : 15));
    } else
        r = -1;
    if (cnt == 0) {
        if (r == 0 && alt[0] != 0) {
            snprintf(b, sizeof b, "(0,%s)", alt);
            word = (word & (mut == M_MASK80 ? 0x80FFu : 0x81FFu)) | (uint32_t)(mut == M_SET50 ? 50 : 51) << 9;
            alt_on = 1;
            o->cfg_written = 1;
        } else
            snprintf(b, sizeof b, "(0,%s)", wav);
    } else {
        snprintf(b, sizeof b, "(0,%s)", wav);
        word = (word & (mut == M_MASK80 ? 0x80FFu : 0x81FFu)) | (cnt - 1) << 9;
        o->cfg_written = mut != M_NOWRITE_DEC;
    }
    MB(0xF0980u) = (uint8_t)word;
    MB(0xF0981u) = (uint8_t)(word >> 8);
    for (i = 0; i < 4; i++)
        MB(0x11AED8u + (uint32_t)i) = (uint8_t)(i == 0 ? alt_on : 0);
    if (o->cfg_written)
        memcpy(o->cfg, &MB(0xF097Cu), 6);
    strcat(o->snd, b);
    if (anim) {
        flc = entry(&g_ct4, tab(0xCAAA0u, id), &flen);
        x = (0x270 - le16s(flc + 8)) / 2 + 8 + (mut == M_CENTRE);
        y = (0x13F - le16s(flc + 10)) / 2 + 6;
        strcpy(o->mouse, "HS");
        snprintf(o->ibuf, sizeof o->ibuf, "(blk,%X)(F522C,FF00)", (unsigned)flen);
        {
            const fcache *c = model_frames(flc, flen, x, y);
            o->nf = c->nf;
            memcpy(o->fr, c->fr, sizeof o->fr);
            o->final = c->final;
        }
    } else {
        start_screen();
        o->final = screen_hash();
    }
    if (digital && alt_on) {
        for (i = 0; i < 4; i++)
            MB(0x11AED8u + (uint32_t)i) = 0;
        if (mut != M_NO_RELOAD) {
            snprintf(b, sizeof b, "(0,%s)", wav);
            strcat(o->snd, b);
        }
    }
    o->ds = m;
    o->dsh = fnv(m, DS_SIZE, 1);
#undef MB
#undef MW
#undef MD
}

/* ---- running the port --------------------------------------------------------------------------------------------- */

static void reset_traces(void)
{
    g_snd[0] = g_mouse[0] = g_ibuf[0] = 0;
    g_rand = 0;
}

static void read_cfg(outcome *o)
{
    FILE *fp = fopen("aciv.cfg", "rb");
    o->cfg_written = 0;
    if (fp != NULL) {
        o->cfg_written = (int)fread(o->cfg, 1, 6, fp) == 6 && fgetc(fp) == EOF ? 1 : 2;   /* 2: not 6 bytes */
        fclose(fp);
    }
}

static void collect(outcome *o)
{
    o->rand = g_rand;
    strcpy(o->snd, g_snd);
    strcpy(o->mouse, g_mouse);
    strcpy(o->ibuf, g_ibuf);
    read_cfg(o);
    o->ds = g_ds;
    o->dsh = fnv(g_ds, DS_SIZE, 1);
    o->nf = g_nf;
    memcpy(o->fr, g_fr, sizeof o->fr);
    o->final = screen_hash();
}

static void port_run(void (*fn)(void), outcome *o)
{
    remove("aciv.cfg");
    start_screen();
    reset_traces();
    g_cap = 1;
    g_nf = 0;
    fn();
    g_cap = 0;
    collect(o);
    remove("aciv.cfg");
}

static int g_id;
static void call_7A0A0(void) { f7A0A0(g_id); }
static void call_7A4C4(void) { f7A4C4(g_id); }   /* the argument is never read */

/* the differences between two outcomes; 0 when equal (printed unless quiet) */
static int differ(const char *name, const outcome *p, const outcome *m)
{
    int n = 0;
    uint32_t i;
    if (p->rand != m->rand) { n++; if (!g_quiet) printf("  %s: game_rand %d, model %d\n", name, p->rand, m->rand); }
    if (strcmp(p->snd, m->snd)) { n++; if (!g_quiet) printf("  %s: f7B504 '%s', model '%s'\n", name, p->snd, m->snd); }
    if (strcmp(p->mouse, m->mouse)) { n++; if (!g_quiet) printf("  %s: mouse '%s', model '%s'\n", name, p->mouse, m->mouse); }
    if (strcmp(p->ibuf, m->ibuf)) { n++; if (!g_quiet) printf("  %s: f8240F '%s', model '%s'\n", name, p->ibuf, m->ibuf); }
    if (p->cfg_written != m->cfg_written || (p->cfg_written && memcmp(p->cfg, m->cfg, 6))) {
        n++;
        if (!g_quiet)
            printf("  %s: aciv.cfg %s %02X%02X%02X%02X%02X%02X, model %s %02X%02X%02X%02X%02X%02X\n", name,
                   p->cfg_written ? "written" : "absent", p->cfg[0], p->cfg[1], p->cfg[2], p->cfg[3], p->cfg[4],
                   p->cfg[5], m->cfg_written ? "written" : "absent", m->cfg[0], m->cfg[1], m->cfg[2], m->cfg[3],
                   m->cfg[4], m->cfg[5]);
    }
    if (p->dsh != m->dsh) {
        n++;
        for (i = 0; !g_quiet && p->ds != NULL && m->ds != NULL && i < DS_SIZE; i++)
            if (p->ds[i] != m->ds[i]) {
                printf("  %s: data byte 0x%X = %02X, model %02X\n", name, (unsigned)(i + DS_BASE), p->ds[i], m->ds[i]);
                break;
            }
    }
    if (p->nf != m->nf || memcmp(p->fr, m->fr, sizeof(uint64_t) * (size_t)(p->nf < MAXF ? p->nf : MAXF))) {
        n++;
        if (!g_quiet)
            printf("  %s: %d frames, model %d (or a frame differs)\n", name, p->nf, m->nf);
    }
    if (p->final != m->final) { n++; if (!g_quiet) printf("  %s: final screen / DAC differ\n", name); }
    return n;
}

/* ---- the cases of parts A and B ----------------------------------------------------------------------------------- */

typedef struct {
    int id;
    uint8_t cfg0;            /* aciv.cfg byte 0 */
    uint16_t word;           /* aciv.cfg bytes 4..5 */
    uint8_t digital;
    uint32_t last, seed;
} kase;

#define MAXK 1400
static kase g_k[MAXK];
static outcome g_po[MAXK];   /* the port's outcomes, kept for part E */
static int g_nk;
static uint8_t *g_base;      /* the data object as loaded */
static uint8_t *g_m;         /* the model's copy */

static void setup(const kase *k, uint8_t *ds)
{
    uint8_t *save = g_ds;
    g_ds = ds;
    memcpy(ds, g_base, DS_SIZE);
    setD8(0xF097Cu, k->cfg0);
    setD16(0xF0980u, k->word);
    setD8(0xF625Cu, k->digital);
    setD32(0xCAC64u, k->last);
    setD32(0xCAC60u, k->last ^ 0x5A5Au);
    setD32(0xCAC5Cu, k->seed);
    setD32(0x11AED8u, 0);
    setD32(0xF6278u, 1);
    g_ds = save;
}

static uint32_t g_lcg = 12345;
static uint32_t rnd(uint32_t n) { g_lcg = g_lcg * 1103515245u + 12345u; return (g_lcg >> 8) % n; }

/* a seed whose next game_rand gives roll r (r * 10 >> 15) */
static uint32_t seed_for(int r)
{
    for (;;) {
        uint32_t s = rnd(0x7FFFFFFF) * 2u + rnd(2), n = s * 0x15A4E35u + 1u;
        if ((int)((((n >> 16) & 0x7FFF) * 10u) >> 15) == r)
            return s;
    }
}

static void add_case(int id, uint8_t cfg0, int cnt, uint16_t around, uint8_t digital, uint32_t last, uint32_t seed)
{
    kase *k;
    if (g_nk >= MAXK)
        return;
    k = &g_k[g_nk++];
    k->id = id;
    k->cfg0 = cfg0;
    k->word = (uint16_t)((around & 0x81FFu) | (uint32_t)cnt << 9);
    k->digital = digital;
    k->last = last;
    k->seed = seed;
}

static int run_case(int i, int mut, int keep)
{
    char name[96];
    outcome m;
    int d;
    const kase *k = &g_k[i];
    if (keep) {
        setup(k, g_ds);
        g_id = k->id;
        port_run(call_7A0A0, &g_po[i]);
    }
    setup(k, g_m);
    model_7A0A0(mut, g_m, k->id, &m);
    snprintf(name, sizeof name, "case %d (id 0x%X cfg0 %02X word %04X dig %d last %X)", i, k->id, k->cfg0, k->word,
             k->digital, (unsigned)k->last);
    d = differ(name, &g_po[i], &m);
    g_po[i].ds = NULL;       /* the live image is reused by the next case: the hash stays */
    return d;
}

/* ---- main --------------------------------------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    const char *catdir = argc > 2 ? argv[2] : "C:/work/advciv_play";
    char path[512];
    int i, id, a_cases, b_cases, pass, fails0, mut;
    FILE *probe;
    uint8_t ibuf[0xFF00];

    probe = fopen("ACIV.CFG", "rb");
    if (probe == NULL)
        probe = fopen("AC_MAIN.EXE", "rb");
    if (probe != NULL) {
        fclose(probe);
        printf("anim_test: run me in a scratch folder (this one holds the game's files)\n");
        return 2;
    }
    if (state_init(gamedir) != 0)
        return 2;
    g_base = malloc(DS_SIZE);
    g_m = malloc(DS_SIZE);
    g_play = malloc(1u << 20);
    if (g_base == NULL || g_m == NULL || g_play == NULL)
        return 2;

    fg_init();
    fg_set_headless(1);
    fg_set_input_hook(hook);
    snprintf(path, sizeof path, "%s/ACIV.CT1", catdir);
    g_ct1 = read_all(path);
    setD32(0xF40D4u, (uint32_t)cat_open_file(path));
    snprintf(path, sizeof path, "%s/ACIV.CT4", catdir);
    g_ct4 = read_all(path);
    setD32(0xF40E0u, (uint32_t)cat_open_file(path));
    if (g_ct1.d == NULL || g_ct4.d == NULL || D32(0xF40D4u) == 0 || D32(0xF40E0u) == 0) {
        printf("anim_test: cannot read the catalogs in %s\n", catdir);
        return 2;
    }
    ui_heap_F522C = ibuf;
    {
        int h = fg_vbdefine(g_bb, 640, 480);
        setD32(0xF52C8u, (uint32_t)h);
    }
    for (i = 0; i < 640 * 480; i++) {
        g_scr0[i] = (uint8_t)(i * 7 + i / 640 * 13);
        g_bb0[i] = (uint8_t)(i * 3 + i / 640 * 5 + 1);
    }
    for (i = 0; i < 768; i++)
        g_dac0[i] = (uint8_t)((i * 5) & 0x3F);
    memcpy(g_base, g_ds, DS_SIZE);

    /* ---- hand checks: the tables and the files the model relies on --------------------------------------------- */
    for (id = 0x12; id <= 0x1E; id++) {
        uint32_t len = 0;
        const uint8_t *f = entry(&g_ct4, tab(0xCAAA0u, id), &len);
        CHECK(f != NULL, "FLC %s of 0x%X not in ACIV.CT4", tab(0xCAAA0u, id), id);
        if (f != NULL) {
            CHECK((f[4] | f[5] << 8) == 0xAF12 && le16s(f + 8) == 220 && le16s(f + 10) == 150,
                  "FLC %s: magic %04X %dx%d", tab(0xCAAA0u, id), f[4] | f[5] << 8, le16s(f + 8), le16s(f + 10));
            CHECK((0x270 - 220) / 2 + 8 == 210 && (0x13F - 150) / 2 + 6 == 90, "position");
        }
        CHECK(strcmp(tab(0xCAA6Cu, id), id <= 0x13 ? "kaboom3.wav" : "") == 0, "alt of 0x%X = '%s'", id,
              tab(0xCAA6Cu, id));
    }
    CHECK(strcmp(tab(0xCAA38u, 0x12), "earthquk.wav") == 0 && strcmp(tab(0xCAA38u, 0x1E), "piracy.wav") == 0,
          "WAV table");

    /* ---- A. the grid --------------------------------------------------------------------------------------------- */
    {
        static const int cnts[6] = { 0, 0, 1, 2, 51, 63 };
        for (id = 0x12; id <= 0x1E; id++)
            for (i = 0; i < 6; i++) {
                int anim, dig;
                for (anim = 0; anim < 2; anim++)
                    for (dig = 0; dig < 2; dig++) {
                        uint8_t cfg0 = (uint8_t)((anim ? 0x04 : 0) | (rnd(2) ? 0x01 : 0) | 0x42);
                        uint32_t seed = i == 0 ? seed_for(0) : seed_for(1 + (int)rnd(9));
                        add_case(id, cfg0, cnts[i], (uint16_t)rnd(0x10000), (uint8_t)dig, 0xFFFFFFFFu, seed);
                    }
                /* already shown */
                add_case(id, 0x47, cnts[i], (uint16_t)rnd(0x10000), 1, (uint32_t)id, seed_for(0));
            }
    }
    a_cases = g_nk;
    fails0 = g_fails;
    pass = 0;
    for (i = 0; i < a_cases; i++) {
        int d = run_case(i, M_NONE, 1);
        g_fails += d != 0;
        pass += d == 0;
    }
    /* hand checks on the files written: kaboom3 sets 0x66 in byte 5 (bits 9..14 = 51), keeping bits 8 and 15 */
    {
        kase k = { 0x13, 0x45, 0x8100 | 0x0042, 1, 0xFFFFFFFFu, 0 };
        outcome o;
        uint8_t e[6];
        k.seed = seed_for(0);
        setup(&k, g_ds);
        g_id = k.id;
        port_run(call_7A0A0, &o);
        memcpy(e, DP(0xF097Cu), 6);
        CHECK(o.cfg_written == 1 && o.cfg[0] == 0x45 && o.cfg[4] == 0x42 && o.cfg[5] == 0xE7 && e[5] == 0xE7,
              "kaboom3 on Volcano: aciv.cfg %d %02X..%02X %02X", o.cfg_written, o.cfg[0], o.cfg[4], o.cfg[5]);
        CHECK(strcmp(o.snd, "(0,kaboom3.wav)(0,volcano.wav)") == 0 && o.rand == 1, "kaboom3 sounds '%s'", o.snd);
        CHECK(o.nf > 1 && strcmp(o.mouse, "HS") == 0, "volcano.flc frames %d mouse %s", o.nf, o.mouse);
        k.word = 0x0300;     /* counter 1, bit 8 set */
        setup(&k, g_ds);
        port_run(call_7A0A0, &o);
        CHECK(o.cfg_written == 1 && o.cfg[4] == 0x00 && o.cfg[5] == 0x01 && o.rand == 0,
              "count-down 1 -> 0: %02X %02X rand %d", o.cfg[4], o.cfg[5], o.rand);
        k.word = 0x0000;
        k.id = 0x14;         /* Treachery: roll 0, no alternative */
        g_id = k.id;
        setup(&k, g_ds);
        port_run(call_7A0A0, &o);
        CHECK(o.cfg_written == 0 && o.rand == 1 && strcmp(o.snd, "(0,treach.wav)") == 0,
              "Treachery roll 0: cfg %d rand %d '%s'", o.cfg_written, o.rand, o.snd);
    }
    printf("A. 0x7A0A0 grid: %d of %d cases agree with the model (13 calamities x counters 0/roll 0, 0/roll 1..9, 1, 2, "
           "51, 63 x animations x digital sound, + already shown); hand checks %s\n", pass, a_cases,
           g_fails == fails0 + (a_cases - pass) ? "pass" : "FAIL");

    /* ---- B. random states ---------------------------------------------------------------------------------------- */
    for (i = 0; i < 500; i++) {
        int cnt = rnd(3) == 0 ? (int)rnd(64) : 0;
        int r = (int)rnd(4) == 0 ? 0 : (int)rnd(10);
        id = 0x12 + (int)rnd(13);
        add_case(id, (uint8_t)((rnd(4) == 0 ? 0x04 : 0) | (rnd(0x100) & 0xFB)), cnt, (uint16_t)rnd(0x10000),
                 (uint8_t)(rnd(3) ? 1 : 0), rnd(5) == 0 ? (uint32_t)id : rnd(3) ? 0xFFFFFFFFu : 0x12u + rnd(13),
                 seed_for(r));
    }
    b_cases = g_nk - a_cases;
    pass = 0;
    for (i = a_cases; i < g_nk; i++) {
        int d = run_case(i, M_NONE, 1);
        g_fails += d != 0;
        pass += d == 0;
    }
    printf("B. 0x7A0A0 random states: %d of %d agree\n", pass, b_cases);

    /* ---- C. 0x7A4C4 / 0x7A4A0 ------------------------------------------------------------------------------------ */
    {
        kase k = { 0x12, 0x47, 0, 1, 0xFFFFFFFFu, 0 };
        outcome o, m;
        int c0 = g_fails;
        k.seed = seed_for(3);
        setup(&k, g_ds);
        setD32(0xF783Cu, 0x19);   /* Flood */
        g_id = 2000;
        port_run(call_7A4C4, &o);
        setup(&k, g_m);
        g_m[0xF783Cu - DS_BASE] = 0x19;
        model_7A0A0(M_NONE, g_m, 0x19, &m);
        CHECK(differ("0x7A4C4 on", &o, &m) == 0, "0x7A4C4 with animations on");
        CHECK(strcmp(o.snd, "(0,flood.wav)") == 0 && o.nf > 1, "0x7A4C4: '%s' %d frames", o.snd, o.nf);
        /* again: already shown, nothing; after 0x7A4A0 it plays again */
        reset_traces();
        f7A4C4(-600);
        CHECK(g_rand == 0 && g_snd[0] == 0 && g_mouse[0] == 0, "0x7A4C4 twice: '%s' rand %d", g_snd, g_rand);
        f7A4A0();
        CHECK(S32(0xCAC60u) == -1 && S32(0xCAC64u) == -1, "0x7A4A0");
        reset_traces();
        f7A4C4(-600);
        CHECK(g_rand == 1 && strcmp(g_snd, "(0,flood.wav)") == 0, "0x7A4C4 after 0x7A4A0: '%s'", g_snd);
        remove("aciv.cfg");
        /* animations off: nothing at all, not even the roll, and [0xCAC64] untouched */
        k.cfg0 = 0x43;
        setup(&k, g_ds);
        setD32(0xF783Cu, 0x12);
        port_run(call_7A4C4, &o);
        CHECK(o.rand == 0 && o.snd[0] == 0 && o.mouse[0] == 0 && o.nf == 0 && o.cfg_written == 0 &&
              S32(0xCAC64u) == -1, "0x7A4C4 with animations off did something");
        printf("C. 0x7A4C4 / 0x7A4A0: %s\n", g_fails == c0 ? "pass" : "FAIL");
    }

    /* ---- D. Zeus and the BRPC ------------------------------------------------------------------------------------- */
    {
        int c0 = g_fails;
        uint32_t len = 0, glen = 0;
        const uint8_t *z = entry(&g_ct4, "zeus.flc", &len), *g = entry(&g_ct1, "brpc.gif", &glen);
        const fcache *c;
        gif_image gi;
        int x, y, r;

        CHECK(z != NULL && g != NULL, "zeus.flc / brpc.gif not in the catalogs");
        if (z != NULL && g != NULL) {
            /* Zeus: sound first, the FLC at (210, 90), the area put back, nothing written to the data object */
            memcpy(g_ds, g_base, DS_SIZE);
            setD8(0xF097Cu, 0x47);
            setD8(0xF625Cu, 1);
            memcpy(g_m, g_ds, DS_SIZE);
            remove("aciv.cfg");
            start_screen();
            reset_traces();
            g_cap = 1;
            g_nf = 0;
            f79ADC(DSTR(0xB9561u), "zeus.wav");
            g_cap = 0;
            {
                int nf = g_nf;
                uint64_t fin = screen_hash();
                uint64_t fr[MAXF];
                memcpy(fr, g_fr, sizeof fr);
                x = (0x270 - le16s(z + 8)) / 2 + 8;
                y = (0x13F - le16s(z + 10)) / 2 + 6;
                c = model_frames(z, len, x, y);
                CHECK(nf == c->nf && memcmp(fr, c->fr, sizeof(uint64_t) * (size_t)nf) == 0 && nf > 1,
                      "Zeus frames: %d, model %d", nf, c->nf);
                CHECK(fin == c->final, "Zeus: the screen not restored");
                CHECK(strcmp(g_snd, "(0,zeus.wav)") == 0 && strcmp(g_mouse, "HSHS") == 0, "Zeus traces '%s' '%s'",
                      g_snd, g_mouse);
                snprintf(path, sizeof path, "(blk,%X)(F522C,FF00)", (unsigned)len);
                CHECK(strcmp(g_ibuf, path) == 0, "Zeus f8240F '%s'", g_ibuf);
                CHECK(memcmp(g_ds, g_m, DS_SIZE) == 0 && g_rand == 0, "Zeus wrote the data object or rolled");
                probe = fopen("aciv.cfg", "rb");
                CHECK(probe == NULL, "Zeus wrote aciv.cfg");
                if (probe != NULL)
                    fclose(probe);
            }
            /* the BRPC: the picture on the screen at (160, 65), the back buffer and the palette as they were */
            r = gif_decode(g, glen, &gi);
            CHECK(r == 0 && gi.w == 320 && gi.h == 200, "brpc.gif %d %dx%d", r, gi.w, gi.h);
            CHECK(g[10] == 0xF7 && g[13 + 768] == 0x2C, "brpc.gif: the image descriptor right after the table");
            if (r == 0) {
                int ok = 1, yy, xx;
                const uint8_t *s;
                x = (0x270 - le16s(g + 13 + 768 + 5)) / 2 + 8;
                y = (0x13F - le16s(g + 13 + 768 + 7)) / 2 + 6;
                CHECK(x == 160 && y == 65, "BRPC at (%d, %d)", x, y);
                for (pass = 0; pass < 2; pass++) {   /* animations on (0x794BE) and off (0x794E9): the same routine */
                    start_screen();
                    reset_traces();
                    g_nf = 0;
                    g_cap = 1;
                    f79C70(DSTR(pass ? 0xB9574u : 0xB956Au), DSTR(0xB9573u));
                    g_cap = 0;
                    s = fg_screen();
                    ok = 1;
                    for (yy = 0; yy < 480; yy++)
                        for (xx = 0; xx < 640; xx++) {
                            int in = xx >= x && xx < x + 320 && yy >= y && yy < y + 200;
                            uint8_t want = in ? gi.pixels[(yy - y) * 320 + (xx - x)] : g_scr0[yy * 640 + xx];
                            if (s[yy * 640 + xx] != want)
                                ok = 0;
                        }
                    CHECK(ok, "BRPC: the screen is not the picture at (160, 65) over the old screen");
                    CHECK(memcmp(g_bb, g_bb0, sizeof g_bb) == 0, "BRPC: the back buffer not restored");
                    CHECK(memcmp(fg_dac(), g_dac0, 768) == 0, "BRPC: the palette changed");
                    CHECK(strcmp(g_snd, "(0,)") == 0 && strcmp(g_mouse, "HSHS") == 0 && g_nf == 0,
                          "BRPC traces '%s' '%s' %d", g_snd, g_mouse, g_nf);
                    CHECK(memcmp(g_ds, g_m, DS_SIZE) == 0 && g_rand == 0, "BRPC wrote the data object or rolled");
                }
                gif_free(&gi);
            }
        }
        printf("D. 0x79ADC Zeus / 0x79C70 BRPC: %s\n", g_fails == c0 ? "pass" : "FAIL");
    }

    /* ---- E. mutations: each broken model must disagree with the port somewhere in parts A and B ------------------ */
    g_quiet = 1;
    fails0 = g_fails;
    {
        int caught = 0;
        for (mut = 1; mut < M_COUNT; mut++) {
            int n = 0;
            for (i = 0; i < g_nk; i++)
                n += run_case(i, mut, 0) != 0;
            if (n > 0)
                caught++;
            else
                printf("  mutation '%s' NOT caught\n", g_mut_name[mut]);
            if (!g_quiet || n == 0)
                continue;
            printf("  mutation '%s': %d of %d cases disagree\n", g_mut_name[mut], n, g_nk);
        }
        g_fails = fails0 + (M_COUNT - 1 - caught);
        printf("E. mutations: %d of %d caught\n", caught, M_COUNT - 1);
    }
    remove("aciv.cfg");
    printf("%s (%d failures)\n", g_fails ? "FAIL" : "PASS", g_fails);
    return g_fails ? 1 : 0;
}
