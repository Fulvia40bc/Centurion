/* The Fastgraph 4.04 / Fastgraph/Image 2.04 / Fastgraph/Fonts 2.04 calls the game makes, for mode 25 (640x480x256)
   only, as docs/LIBRARY_CALLS.md describes them (each routine's address is in its comment; D = read in the library
   code, G = inference). The library is replaced, not translated: its state lives here in C (the game's own code never
   reads Fastgraph's globals at 0xCAF96..0xE0D42, checked: no reference below 0x80000), named after the original
   globals. The game's screen paths are mode index 0x64 (the banked SVGA page) and 0x20 (an open virtual buffer).

   Memory model: Fastgraph draws through linear offsets (row * stride + x) with no clipping in most routines. Where the
   original has no clipping the port computes the same linear offset and drops only bytes outside the target
   surface's memory, so a run past the right edge wraps into the next row as on the 386, and what would have
   scribbled outside the buffer (a memory error in the original) is skipped. */

#include "fg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "catalog.h"
#include "fgfont.h"
#include "gif.h"
#include "palette.h"
#include "platform.h"
#include "state.h"

#define SCR_W 640                 /* mode 25: [0xDA2F0] width, [0xDA100] bytes per line */
#define SCR_H 480                 /* [0xDA36C] */
#define RETRACE_HZ 70             /* the pacing rate the port gives fg_setdacs and the present throttle (G) */

/* ---- surfaces ---------------------------------------------------------------------------------------------- */

typedef struct {
    uint8_t *base;                /* 0xCB196[h] */
    int w, h;                     /* 0xCB296[h], 0xCB216[h] */
    int minx, maxx, miny, maxy;   /* the buffer's own clip: 0xCB096, 0xCAF96, 0xCB116, 0xCB016 [h] */
} fg_vbuf;

static uint8_t s_screen[SCR_W * SCR_H];
static uint8_t s_dac[256][3];
static fg_vbuf s_vb[32];
static int s_active = -1;                     /* 0xD65F8: active vb handle, -1 = the screen */
static uint8_t *s_surf = s_screen;            /* the active surface: [0xD6606] or the video page */
static int s_sw = SCR_W, s_sh = SCR_H;        /* [0xDA2F0], [0xDA36C] of the active surface */
static int s_minx, s_maxx = SCR_W - 1, s_miny, s_maxy = SCR_H - 1;   /* clip 0xD5FAC/0xD5FB0/0xD5FB4/0xD5FB8 */
static int s_scr_minx, s_scr_maxx, s_scr_miny, s_scr_maxy;           /* the screen's clip saved by fg_vbopen (0xD65E0..) */
static uint8_t *s_last_base;                  /* [0xD6606]/[0xD65FE]: the last opened vb, kept after fg_vbclose */
static int s_last_w, s_last_h;
static int s_last_valid;                      /* [0xD65FA] holds the screen path (a vb was opened once) */
static int s_cx, s_cy;                        /* graphics cursor 0xD5F9C/0xD5FA0: one for all surfaces (D) */
static uint8_t s_color;                       /* 0xD5F61 (= low byte of 0xD5F62) */
static int s_boxdx = 1, s_boxdy = 1;          /* 0xD5FA4/0xD5FA8 */
static uint8_t s_trans[256];                  /* 0xD625A: fg_settrans table for fg_vbtccopy */
static int s_dirty = 1;                       /* screen or DAC changed since the last present (port) */
static int s_headless;
static uint64_t s_last_present, s_next_retrace;

static uint8_t s_imgbuf_internal[4096];       /* 0xD6694 */
static uint8_t *s_imgbuf = s_imgbuf_internal; /* [0xD668A] */
static uint32_t s_imgsize = 4096;             /* [0xD6690] */

static char s_gamedir[1024];
static void (*s_quit)(void);

/* the active surface changed where the window can see it */
static void touch(void)
{
    if (s_active < 0)
        s_dirty = 1;
}

/* one byte at a linear offset of a buffer of size bytes; outside it is dropped (see the header comment) */
static void put_lin(uint8_t *b, int64_t size, int64_t off, uint8_t c)
{
    if (off >= 0 && off < size)
        b[off] = c;
}

static int get_lin(const uint8_t *b, int64_t size, int64_t off)
{
    return off >= 0 && off < size ? b[off] : 0;   /* G: the original reads whatever memory is there */
}

static int64_t surf_size(void) { return (int64_t)s_sw * s_sh; }

/* plot on the active surface at (x, y) without a clip test (callers have clipped) */
static void plot(int x, int y, uint8_t c)
{
    put_lin(s_surf, surf_size(), (int64_t)y * s_sw + x, c);
}

/* n bytes forward as rep movsd, movsw, movsb: each chunk is read whole before it is written, so an overlapping copy
   inside one buffer smears as on the 386 (0x87A94-0x87AB0) */
static void copy_fwd(uint8_t *db, int64_t dsize, int64_t doff, const uint8_t *sb, int64_t ssize, int64_t soff, int n)
{
    while (n > 0) {
        uint8_t t[4];
        int k = n >= 4 ? 4 : n >= 2 ? 2 : 1, i;
        for (i = 0; i < k; i++)
            t[i] = (uint8_t)get_lin(sb, ssize, soff + i);
        for (i = 0; i < k; i++)
            put_lin(db, dsize, doff + i, t[i]);
        soff += k;
        doff += k;
        n -= k;
    }
}

/* ---- set-up, screen, presentation -------------------------------------------------------------------------- */

/* the mouse (0xD5FD6 initialised, 0xD5FD7 visibility byte) and the port's view of the platform's pointer */
static int s_mouse_init, s_mouse_vis;
static int s_sys_pointer;                     /* no data object for the cursor masks: the system pointer stands in */
static int s_mx, s_my, s_mb, s_mlatch;        /* position in game pixels, buttons held, presses since the last read */
static int s_pmx = -1, s_pmy = -1, s_pvis;    /* pointer as last presented */

/* the BIOS keyboard buffer (INT 16h): 15 keystrokes, (scan << 8) | ascii; G for the size (the standard 32-byte ring) */
static uint16_t s_keys[15];
static int s_nkeys;
static int s_test_key;                       /* tests (headless): the next fg_intkey returns this key, 0 */

/* fonts (0xE0B02 cell size per slot, nonzero = used; 0xE0A74 current 1..32, 0 none; 0xE0A70 spacing) */
static fg_font s_font[32];
static int s_font_used[32];
static int s_font_cur;
static int s_spacing;

/* catalogs: the DOS file of 0xA22C3 is the whole file in memory here; pos is the DOS file pointer */
#define NCAT 16
typedef struct { int used; uint8_t *data; size_t size; catalog cat; int64_t pos; } fg_cat;
static fg_cat s_cat[NCAT];

void fg_init(void)
{
    memset(s_screen, 0, sizeof s_screen);
    memset(s_dac, 0, sizeof s_dac);
    memset(s_vb, 0, sizeof s_vb);
    s_active = -1;
    s_surf = s_screen;
    s_sw = SCR_W;
    s_sh = SCR_H;
    s_minx = 0;                               /* fg_setmode 0x80E9A-0x80ECB: the whole page */
    s_maxx = SCR_W - 1;
    s_miny = 0;
    s_maxy = SCR_H - 1;
    s_last_base = NULL;
    s_last_w = s_last_h = 0;
    s_last_valid = 0;
    s_cx = s_cy = 0;
    s_color = 0;
    s_boxdx = s_boxdy = 1;                    /* 0x80D31/0x80D36 */
    memset(s_trans, 0, sizeof s_trans);       /* 0x80E5D: no colour transparent */
    s_imgbuf = s_imgbuf_internal;
    s_imgsize = sizeof s_imgbuf_internal;
    s_mouse_init = s_mouse_vis = 0;
    s_nkeys = 0;
    s_dirty = 1;
    plat_pointer(0);                          /* no pointer on the DOS screen until the game shows Fastgraph's */
}

uint8_t *fg_screen(void)
{
    s_dirty = 1;                              /* the caller may write */
    return s_screen;
}

const uint8_t (*fg_dac(void))[3]
{
    return (const uint8_t (*)[3])s_dac;
}

void fg_set_headless(int on) { s_headless = on; }
void fg_set_quit(void (*handler)(void)) { s_quit = handler; }

void fg_set_gamedir(const char *dir)
{
    snprintf(s_gamedir, sizeof s_gamedir, "%s", dir != NULL ? dir : "");
}

/* Fastgraph's own SVGA pointer (0x81B35, drawn by the mouse event handler): the default masks of 0xCB356 (16 words,
   AND) and 0xCB376 (16 words, XOR), never replaced by the game (no fg_mouseptr call; mouseini converts them at
   0x817BB into bytes of 0x00/0xFF). Each pixel = (pixel AND smask) XOR cmask: an arrow outlined in colour 0 and filled
   with 255 (D, the bytes in the data object). Left edge = (cx - [0xD5FE0]) >> [0xD5FE8] = one pixel left of the mouse
   x, top = the mouse y (D 0x81C31-0x81C6D; [0xD5FE0] = 1 << shift; the shift 1 comes from the driver's position after
   the reset in mode 13h, G). Column 0 of the masks is transparent, so the tip is at the mouse position. The original
   clips left, top and right (0xCB39C = 640) but not the bottom; the port clips all sides. It is drawn into a copy of
   the screen, so the game's pixels never contain it. */
static void draw_pointer(uint8_t *fb)
{
    int r, b, left = s_mx - 1, top = s_my;

    for (r = 0; r < 16; r++) {
        uint16_t sm = D16(0xCB356u + 2u * (uint32_t)r), cm = D16(0xCB376u + 2u * (uint32_t)r);
        int y = top + r;
        if (y < 0 || y >= SCR_H)
            continue;
        for (b = 0; b < 16; b++) {
            int x = left + b;
            uint8_t and_ = (uint8_t)((sm >> (15 - b) & 1) ? 0xFF : 0x00), xor_ = (uint8_t)((cm >> (15 - b) & 1) ? 0xFF : 0x00);
            if (x < 0 || x >= SCR_W)
                continue;
            fb[y * SCR_W + x] = (uint8_t)((fb[y * SCR_W + x] & and_) ^ xor_);   /* 0x81DB5 and al,[esi]; xor al,[esi+1] */
        }
    }
}

static int pointer_drawn(void)
{
    return s_mouse_init && s_mouse_vis != 0 && !s_sys_pointer;   /* 0x81B48: drawn while [0xD5FD7] != 0 */
}

void fg_frame_with_pointer(uint8_t *out, int x, int y)
{
    int sx = s_mx, sy = s_my;

    memcpy(out, s_screen, (size_t)SCR_W * SCR_H);
    s_mx = x;
    s_my = y;
    if (pointer_drawn())
        draw_pointer(out);
    s_mx = sx;
    s_my = sy;
}

void fg_present(void)
{
    static uint8_t frame[SCR_W * SCR_H];
    uint8_t rgb[256][3];
    const uint8_t *fb = s_screen;

    if (pointer_drawn()) {
        memcpy(frame, s_screen, sizeof frame);
        draw_pointer(frame);
        fb = frame;
    }
    pal_dac_to_rgb((const uint8_t (*)[3])s_dac, rgb);
    plat_present(fb, (const uint8_t (*)[3])rgb);
    s_dirty = 0;
    s_pmx = s_mx;
    s_pmy = s_my;
    s_pvis = pointer_drawn();
    s_last_present = plat_ticks_ms();
}

static void push_key(uint8_t scan, uint8_t ascii)
{
    if (scan == 0x57 || scan == 0x58)         /* F11/F12: INT 16h AH=00h only returns standard keys (D 0x87E82, G for these) */
        return;
    if (s_nkeys < (int)(sizeof s_keys / sizeof s_keys[0]))
        s_keys[s_nkeys++] = (uint16_t)(scan << 8 | ascii);   /* a full BIOS buffer drops the key (and beeps) */
}

static void poll_events(void)
{
    plat_event ev;

    while (plat_poll(&ev)) {
        switch (ev.type) {
        case PLAT_KEY:
            push_key(ev.scan, ev.ascii);
            break;
        case PLAT_MOUSE_MOVE:
            s_mx = ev.x;
            s_my = ev.y;
            break;
        case PLAT_MOUSE_DOWN:
            s_mx = ev.x;
            s_my = ev.y;
            s_mb |= ev.button;
            s_mlatch |= ev.button;            /* a press and release between two reads still reads as down once (port) */
            break;
        case PLAT_MOUSE_UP:
            s_mx = ev.x;
            s_my = ev.y;
            s_mb &= ~ev.button;
            break;
        case PLAT_QUIT:
            if (s_quit != NULL)
                s_quit();
            plat_shutdown();
            exit(0);
        default:
            break;
        }
    }
}

/* what the input calls do first (port): show the screen if it, the palette or the pointer changed (at most RETRACE_HZ
   times a second, as the window's present waits for vsync), then take the platform's events */
static void (*s_input_hook)(void);

void fg_test_mouse(int x, int y, int buttons)
{
    s_mx = x;
    s_my = y;
    s_mlatch |= buttons & ~s_mb;   /* a new press reads as down once, as a real one does */
    s_mb = buttons;
}

void fg_set_input_hook(void (*hook)(void))
{
    s_input_hook = hook;
}

void fg_test_key(int key)
{
    s_test_key = key;
}

static void service(void)
{
    int ptr = pointer_drawn();

    if (s_input_hook != NULL)
        s_input_hook();   /* tests: the game is about to read input, its screen is complete */

    if (s_dirty || ptr != s_pvis || (ptr && (s_mx != s_pmx || s_my != s_pmy))) {
        if (s_headless || plat_ticks_ms() - s_last_present >= 1000u / RETRACE_HZ)
            fg_present();
    }
    poll_events();
}

/* ---- files (the original opens relative to the current directory, the game folder) --------------------------- */

static int is_absolute(const char *p)
{
    return p[0] == '/' || p[0] == '\\' || (p[0] != 0 && p[1] == ':');
}

static uint8_t *load_file(const char *name, size_t *len)
{
    char path[1100];
    FILE *f = NULL;
    uint8_t *buf;
    long n;

    if (s_gamedir[0] != 0 && !is_absolute(name)) {
        snprintf(path, sizeof path, "%s/%s", s_gamedir, name);
        f = fopen(path, "rb");
    }
    if (f == NULL)
        f = fopen(name, "rb");
    if (f == NULL)
        return NULL;
    if (fseek(f, 0, SEEK_END) != 0 || (n = ftell(f)) < 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }
    buf = malloc(n > 0 ? (size_t)n : 1);
    if (buf == NULL || fread(buf, 1, (size_t)n, f) != (size_t)n) {
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *len = (size_t)n;
    return buf;
}

/* ---- virtual buffers ----------------------------------------------------------------------------------------- */

/* 0x82398 fg_vbdefine: the first slot whose address is 0 (loopne over 0xCB196); clip 0..w-1, 0..h-1 */
int fg_vbdefine(uint8_t *buffer, int width, int height)
{
    int h;

    for (h = 0; h < 32; h++)
        if (s_vb[h].base == NULL)
            break;
    if (h == 32)
        return -1;                            /* 0x82408 */
    s_vb[h].base = buffer;
    s_vb[h].w = width;
    s_vb[h].h = height;
    s_vb[h].maxx = width - 1;                 /* 0x823D8-0x82402 */
    s_vb[h].maxy = height - 1;
    s_vb[h].minx = 0;
    s_vb[h].miny = 0;
    return h;
}

/* 0x87C31 fg_vbopen */
int fg_vbopen(int handle)
{
    if ((unsigned)handle > 31)                /* 0x87C3A unsigned ja */
        return -1;
    if (s_vb[handle].base == NULL)            /* 0x87C4D */
        return -2;
    if (s_active < 0) {                       /* save the screen's state only when no vb is open (0x87C51-0x87C9B) */
        s_scr_minx = s_minx;
        s_scr_maxx = s_maxx;
        s_scr_miny = s_miny;
        s_scr_maxy = s_maxy;
        s_last_valid = 1;                     /* [0xD65FA] = the screen's mode index */
    }
    s_active = handle;
    s_surf = s_vb[handle].base;
    s_sw = s_vb[handle].w;
    s_sh = s_vb[handle].h;
    s_last_base = s_surf;                     /* [0xD6606], [0xD65FE] */
    s_last_w = s_sw;
    s_last_h = s_sh;
    s_minx = s_vb[handle].minx;               /* the vb's own clip (0x87CE6-0x87D19) */
    s_maxx = s_vb[handle].maxx;
    s_miny = s_vb[handle].miny;
    s_maxy = s_vb[handle].maxy;
    return 0;                                 /* cursor, colour and DAC unchanged */
}

/* 0x87D73 fg_vbclose */
void fg_vbclose(void)
{
    if (s_active < 0)
        return;
    s_surf = s_screen;
    s_sw = SCR_W;
    s_sh = SCR_H;
    s_minx = s_scr_minx;
    s_maxx = s_scr_maxx;
    s_miny = s_scr_miny;
    s_maxy = s_scr_maxy;
    s_active = -1;                            /* the vb descriptor [0xD6606]/[0xD65FE] stays (fg_vbpaste uses it) */
}

/* 0x87A1A fg_vbcopy: lower-left corner of the target at (newx, newy); no clipping, forward copy */
int fg_vbcopy(int minx, int maxx, int miny, int maxy, int newx, int newy, int source, int dest)
{
    const fg_vbuf *s = &s_vb[source & 0x1F], *d = &s_vb[dest & 0x1F];
    int w = maxx - minx + 1, h = maxy - miny + 1, r;

    if (s->base == NULL || d->base == NULL)
        return 0;
    if (w <= 0 || h <= 0)                     /* the original loops ~2^32 times or copies garbage: nothing here (G) */
        return 0;
    for (r = 0; r < h; r++)
        copy_fwd(d->base, (int64_t)d->w * d->h, ((int64_t)newy + miny - maxy + r) * d->w + newx,
                 s->base, (int64_t)s->w * s->h, ((int64_t)miny + r) * s->w + minx, w);
    return 0;
}

/* 0x8A085 fg_vbpaste: rectangle of the last opened vb -> the screen, lower-left at (x, y); dispatched on [0xD65FA],
   so it writes the screen even with a vb open; no clipping */
int fg_vbpaste(int minx, int maxx, int miny, int maxy, int x, int y)
{
    int w = maxx + 1 - minx, h = maxy + 1 - miny, r;

    if (!s_last_valid || s_last_base == NULL || w <= 0 || h <= 0)   /* G: stale [0xD65FA] / bad sizes */
        return 0;
    for (r = 0; r < h; r++)
        copy_fwd(s_screen, (int64_t)SCR_W * SCR_H, ((int64_t)y + miny - maxy + r) * SCR_W + x,
                 s_last_base, (int64_t)s_last_w * s_last_h, ((int64_t)miny + r) * s_last_w + minx, w);
    s_dirty = 1;
    return 0;
}

/* 0x90940 fg_vbcut: rectangle of the screen -> the last opened vb, lower-left at (newx, newy); no clipping */
int fg_vbcut(int minx, int maxx, int miny, int maxy, int newx, int newy)
{
    int w = maxx + 1 - minx, h = maxy + 1 - miny, r;

    if (!s_last_valid || s_last_base == NULL || w <= 0 || h <= 0)   /* G, as fg_vbpaste */
        return 0;
    for (r = 0; r < h; r++)
        copy_fwd(s_last_base, (int64_t)s_last_w * s_last_h, ((int64_t)newy + miny - maxy + r) * s_last_w + newx,
                 s_screen, (int64_t)SCR_W * SCR_H, ((int64_t)miny + r) * SCR_W + minx, w);
    return 0;
}

/* 0x8DF22 fg_vbtccopy: as fg_vbcopy, skipping pixels whose colour is marked in the fg_settrans table */
int fg_vbtccopy(int minx, int maxx, int miny, int maxy, int newx, int newy, int source, int dest)
{
    const fg_vbuf *s = &s_vb[source & 0x1F], *d = &s_vb[dest & 0x1F];
    uint8_t trans[256];
    int w = maxx + 1 - minx, h = maxy + 1 - miny, r, c;

    if (s->base == NULL || d->base == NULL)
        return 0;
    memcpy(trans, s_trans, sizeof trans);     /* 0x8DF2x: the table is copied to the stack first */
    for (r = 0; r < h; r++)
        for (c = 0; c < w; c++) {
            int p = get_lin(s->base, (int64_t)s->w * s->h, ((int64_t)miny + r) * s->w + minx + c);
            if (!trans[p])                    /* 0x8DFC6 xlatb; jne skip */
                put_lin(d->base, (int64_t)d->w * d->h, ((int64_t)newy + miny - maxy + r) * d->w + newx + c, (uint8_t)p);
        }
    return 0;
}

/* 0x8DEE1 (Fastgraph's fg_tcdefine): colour number transparent for fg_vbtccopy when flag's low 16 bits are nonzero.
   The 16-colour mask word 0xD5F92 it also updates has no use in mode 25 and is not kept. */
int fg_settrans(int number, int flag)
{
    s_trans[number & 0xFF] = (uint8_t)((flag | flag >> 8) & 0xFF);   /* 0x8DEF1 or al,ah */
    return 0;
}

/* ---- drawing ------------------------------------------------------------------------------------------------- */

/* 0x87AC2 fg_setcolor, mode 25 / vb path 0x87B99 */
int fg_setcolor(int color)
{
    s_color = (uint8_t)(color & 0xFF);
    return 0;
}

int fg_getcolor(void) { return s_color; }

/* 0x87DF7 fg_move: stored as given */
void fg_move(int x, int y)
{
    s_cx = x;
    s_cy = y;
}

int fg_getxpos(void) { return s_cx; }        /* 0x9E010 */
int fg_getypos(void) { return s_cy; }        /* 0x9E016 */

static int in_clip(int x, int y)
{
    return x >= s_minx && x <= s_maxx && y >= s_miny && y <= s_maxy;   /* 0x90FCC-0x90FF2, inclusive */
}

/* 0x90FC7 fg_point: clipped, opaque */
int fg_point(int x, int y)
{
    if (in_clip(x, y)) {
        plot(x, y, s_color);
        touch();
    }
    return 0;
}

/* 0x914BC fg_getpixel: -1 outside the clip region */
int fg_getpixel(int x, int y)
{
    if (!in_clip(x, y))
        return -1;                            /* 0x914FD mov ax,0xFFFF; cwde */
    return get_lin(s_surf, surf_size(), (int64_t)y * s_sw + x);
}

/* the sign of a - b as cmp/lahf/shl ah sees it (SF of the 32-bit difference) */
static unsigned sf(int a, int b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b) < 0 ? 1u : 0u;
}

/* Cohen-Sutherland outcode of fg_draw (0x8A848): bit 0 x < minx, 1 y < miny, 2 x > maxx, 3 y > maxy */
static unsigned outcode(int x, int y)
{
    return sf(s_maxy, y) << 3 | sf(s_maxx, x) << 2 | sf(y, s_miny) << 1 | sf(x, s_minx);
}

/* imul dx; idiv cx; cwde: (int16)a * (int16)b / (int16)c, 16-bit quotient truncated toward 0 */
static int muldiv16(int a, int b, int c)
{
    int32_t p = (int32_t)(int16_t)a * (int16_t)b, q;

    if ((int16_t)c == 0)
        return 0;                             /* divide error in the original; not reachable (the codes differ), G */
    q = p / (int16_t)c;
    if (q < -32768 || q > 32767)
        return 0;                             /* the same (#DE on a quotient overflow), G */
    return (int16_t)q;
}

/* 0x8A833 fg_draw: line from the cursor to (x, y), clipped; the cursor ends at (x, y) in every case */
int fg_draw(int x, int y)
{
    int qx = x, qy = y;                       /* [ebp+8], [ebp+0xC]; the originals are pushed at 0x8A83A */

    for (;;) {                                /* 0x8A840 */
        unsigned cq = outcode(qx, qy), cp = outcode(s_cx, s_cy), t;
        int sx, sy, ex, dx, dy, step;
        int64_t off, size;

        if ((cq | cp) == 0) {                 /* 0x8A8AC: inside, draw */
            /* 0x8A9D0: start at the end with the smaller y (the cursor when equal) */
            sy = s_cy;
            dy = qy - s_cy;
            if (dy < 0) {
                sy = qy;
                ex = s_cx;
                s_cx = qx;
                dy = -dy;
            } else {
                ex = qx;
            }
            sx = s_cx;
            dx = ex - sx;
            step = 1;
            if (dx < 0) {                     /* 0x8BE6B: x decreasing */
                dx = -dx;
                step = -1;
            }
            size = surf_size();
            off = (int64_t)sy * s_sw + sx;    /* 0x8BDD9 mul [0xD65FE] */
            if ((unsigned)dx >= (unsigned)dy) {   /* cmp ebx,esi; jb: x-major */
                int d = 2 * dy - dx, i;
                for (i = 0; i < dx; i++) {
                    put_lin(s_surf, size, off, s_color);
                    if (d >= 0) {
                        off += s_sw;
                        d += 2 * dy - 2 * dx;
                    } else {
                        d += 2 * dy;
                    }
                    off += step;
                }
                put_lin(s_surf, size, off, s_color);
            } else {                          /* y-major */
                int d = 2 * dx - dy, i;
                put_lin(s_surf, size, off, s_color);
                for (i = 0; i < dy; i++) {
                    if (d >= 0) {
                        off += step;
                        d += 2 * dx - 2 * dy;
                    } else {
                        d += 2 * dx;
                    }
                    off += s_sw;
                    put_lin(s_surf, size, off, s_color);
                }
            }
            touch();
            break;
        }
        if (cq & cp)                          /* 0x8A8B0 test bl,bh: rejected */
            break;
        if (cp == 0) {                        /* 0x8A8C0: clip the end that is outside */
            t = (unsigned)qx; qx = s_cx; s_cx = (int)t;
            t = (unsigned)qy; qy = s_cy; s_cy = (int)t;
            cp = cq;
        }
        if (cp & 1) {                         /* 0x8A8D8 */
            s_cy += muldiv16(qy - s_cy, s_minx - s_cx, qx - s_cx);
            s_cx = s_minx;
        } else if (cp & 2) {                  /* 0x8A916 */
            s_cx += muldiv16(qx - s_cx, s_miny - s_cy, qy - s_cy);
            s_cy = s_miny;
        } else if (cp & 4) {                  /* 0x8A954 */
            s_cy += muldiv16(qy - s_cy, s_maxx - s_cx, qx - s_cx);
            s_cx = s_maxx;
        } else {                              /* 0x8A992 */
            s_cx += muldiv16(qx - s_cx, s_maxy - s_cy, qy - s_cy);
            s_cy = s_maxy;
        }
    }
    s_cx = x;                                 /* 0x8BEC7 pops */
    s_cy = y;
    return 0;
}

/* 0x8C8B2 fg_rect: filled, no clipping */
int fg_rect(int minx, int maxx, int miny, int maxy)
{
    int w = maxx + 1 - minx, h = maxy + 1 - miny, r, c;
    int64_t size = surf_size();

    if (w <= 0 || h <= 0)                     /* the original fills garbage or loops ~2^32 rows: nothing here (G) */
        return 0;
    for (r = 0; r < h; r++)
        for (c = 0; c < w; c++)
            put_lin(s_surf, size, ((int64_t)miny + r) * s_sw + minx + c, s_color);
    touch();
    return 0;
}

/* 0x9D21A fg_clprect: rejected when wholly outside the clip region, else clamped to it and filled */
static int clprect(int minx, int maxx, int miny, int maxy)
{
    if (minx > s_maxx || maxx < s_minx || miny > s_maxy || maxy < s_miny)
        return 0;
    if (minx < s_minx) minx = s_minx;
    if (maxx > s_maxx) maxx = s_maxx;
    if (miny < s_miny) miny = s_miny;
    if (maxy > s_maxy) maxy = s_maxy;
    return fg_rect(minx, maxx, miny, maxy);   /* 0x9D285 */
}

/* 0x8C76E fg_box: four bands of the fg_boxdepth thickness through fg_clprect */
int fg_box(int minx, int maxx, int miny, int maxy)
{
    int t;

    t = miny + s_boxdy - 1;                   /* 0x8C772: top */
    clprect(minx, maxx, miny, t < maxy ? t : maxy);
    t = maxy - s_boxdy + 1;                   /* bottom */
    clprect(minx, maxx, t > miny ? t : miny, maxy);
    t = minx + s_boxdx - 1;                   /* 0x8C7B8: left */
    clprect(minx, t < maxx ? t : maxx, miny, maxy);
    t = maxx - s_boxdx + 1;                   /* right */
    clprect(t > minx ? t : minx, maxx, miny, maxy);
    return 0;
}

/* 0x912BE fg_boxdepth */
int fg_boxdepth(int xdepth, int ydepth)
{
    s_boxdx = xdepth;
    s_boxdy = ydepth;
    return 0;
}

/* 0x9125D fg_setclip: unvalidated; also the open vb's own clip. With no vb open the original also writes entry 255 of
   the per-vb tables (0x91284: the jl on a zero-extended byte is never taken), a scribble over other Fastgraph data the
   port does not reproduce (no game call is affected: both run with a vb open). */
int fg_setclip(int minx, int maxx, int miny, int maxy)
{
    s_minx = minx;
    s_maxx = maxx;
    s_miny = miny;
    s_maxy = maxy;
    if (s_active >= 0) {
        s_vb[s_active].minx = minx;
        s_vb[s_active].maxx = maxx;
        s_vb[s_active].miny = miny;
        s_vb[s_active].maxy = maxy;
    }
    return 0;
}

/* 0x9D830 fg_drawmap: a 1-bit map, its lower-left corner at the cursor, rows stored bottom first, width_bytes bytes a
   row (most significant bit leftmost); set bits in the current colour, clear bits leave the pixel; no clipping
   (vb path 0x9DEF0, SVGA 0x9DD68). Fastgraph/Fonts draws each glyph with it. */
int fg_drawmap(const uint8_t *map, int width_bytes, int rows)
{
    int64_t size = surf_size();
    int r, b;

    if (width_bytes <= 0 || rows <= 0)        /* the original loops ~2^32 times: nothing here (G) */
        return 0;
    for (r = 0; r < rows; r++)
        for (b = 0; b < width_bytes * 8; b++)
            if (map[r * width_bytes + (b >> 3)] & (0x80 >> (b & 7)))   /* dh = 0x80, ror dh */
                put_lin(s_surf, size, ((int64_t)s_cy - r) * s_sw + s_cx + b, s_color);
    touch();
    return 0;
}

/* 0x8E444 fg_getimage: lower-left corner at the cursor, stored bottom row first; no clipping */
int fg_getimage(uint8_t *map, int width, int height)
{
    int64_t size = surf_size();
    int r, c;

    for (r = 0; r < height; r++)
        for (c = 0; c < width; c++)
            map[r * width + c] = (uint8_t)get_lin(s_surf, size, ((int64_t)s_cy - r) * s_sw + s_cx + c);
    return 0;
}

/* 0x8EC48 fg_clpimage / 0x8FA30 fg_flpimage: lower-left corner at the cursor, clipped to the clip region, colour 0
   transparent; flip mirrors left-right in screen space. -1 when wholly clipped. */
static int clp(const uint8_t *map, int width, int height, int flip)
{
    int x = s_cx, y = s_cy, lclip, rclip, bclip = 0, x0, y0 = y, top, w, r, c;
    int64_t size = surf_size();

    lclip = s_minx - x > 0 ? s_minx - x : 0;
    x0 = x > s_minx ? x : s_minx;
    top = y + 1 - height;
    if (top < s_miny)
        height -= s_miny - top;
    if (y > s_maxy) {
        bclip = y - s_maxy;
        height -= bclip;
        y0 = s_maxy;
    }
    if (height <= 0)
        return -1;                            /* 0x8ECDF */
    rclip = x + width - 1 - s_maxx > 0 ? x + width - 1 - s_maxx : 0;
    w = width - rclip - lclip;
    if (w <= 0)
        return -1;
    for (r = 0; r < height; r++)
        for (c = 0; c < w; c++) {
            uint8_t p = flip ? map[(bclip + r) * width + (width - 1 - lclip - c)] : map[(bclip + r) * width + lclip + c];
            if (p)                            /* or al,al; je skip */
                put_lin(s_surf, size, ((int64_t)y0 - r) * s_sw + x0 + c, p);
        }
    touch();
    return 0;
}

int fg_clpimage(const uint8_t *map, int width, int height) { return clp(map, width, height, 0); }
int fg_flpimage(const uint8_t *map, int width, int height) { return clp(map, width, height, 1); }

/* 0x91AB3 fg_dash: Bresenham from the cursor, each point through fg_point when its pattern bit (MSB first) is set */
int fg_dash(int x, int y, int pattern)
{
    int cx = s_cx, cy = s_cy, dx = x - cx, dy = y - cy, sx = dx >= 0 ? 1 : -1, sy = dy >= 0 ? 1 : -1;
    int major, minor, stx, sty, d, i;
    uint16_t pat = (uint16_t)pattern;

    dx = dx < 0 ? -dx : dx;
    dy = dy < 0 ? -dy : dy;
    if (dx >= dy) {                           /* signed jge: x major on a tie */
        major = dx; minor = dy; stx = sx; sty = 0;
    } else {
        major = dy; minor = dx; stx = 0; sty = sy;
    }
    d = 2 * minor - major;
    for (i = 0; i < major; i++) {             /* 0x91B15 */
        pat = (uint16_t)(pat << 1 | pat >> 15);   /* rol ax,1 */
        if (pat & 1)
            fg_point(cx, cy);
        if (d < 0) {
            cx += stx; cy += sty;
            d += 2 * minor;
        } else {
            cx += sx; cy += sy;
            d += 2 * minor - 2 * major;
        }
    }
    s_cx = cx;                                /* 0x91B46: the computed end point, which is (x, y) */
    s_cy = cy;
    pat = (uint16_t)(pat << 1 | pat >> 15);
    if (pat & 1)
        fg_point(cx, cy);
    return 0;
}

/* 0x8C803 fg_dashrel */
int fg_dashrel(int ix, int iy, int pattern)
{
    return fg_dash(s_cx + ix, s_cy + iy, pattern);
}

/* ---- palette ------------------------------------------------------------------------------------------------- */

/* The port's stand-in for the original's wait-until-vertical-retrace (the 3DAh bit-3 test at 0x879EF, also used by
   fg_showflic's palette chunks): show the screen and pace to the next 1/RETRACE_HZ s, except headless. */
static void pace_retrace(void)
{
    if (!s_headless) {                        /* port: one call per retrace period (G for the period) */
        uint64_t now = plat_ticks_ms(), target = s_next_retrace;
        if (target <= now || target > now + 1000u / RETRACE_HZ)
            target = now + 1000u / RETRACE_HZ;   /* the first of a run of calls, or after a pause */
        fg_present();
        now = plat_ticks_ms();
        if (now < target)
            plat_sleep_ms((uint32_t)(target - now));
        s_next_retrace = target + 1000u / RETRACE_HZ;
        poll_events();                        /* the window stays responsive through a fade */
    }
}

/* 0x879DC fg_setdacs: DAC[(start + i) & 255] = values (6 bits kept, as the VGA DAC). The original first waits until
   the VGA is in vertical retrace; the port shows the screen and paces to the next 1/RETRACE_HZ s instead, except
   headless. */
int fg_setdacs(int start, int count, const uint8_t *values)
{
    int i, k;

    if (count <= 0)                           /* jecxz (a negative count would loop ~2^32 times: nothing here, G) */
        return 0;
    for (i = 0; i < count; i++)
        for (k = 0; k < 3; k++)
            s_dac[(start + i) & 0xFF][k] = (uint8_t)(values[3 * i + k] & 0x3F);
    s_dirty = 1;
    pace_retrace();
    return 0;
}

/* 0x90F84 fg_getdacs */
int fg_getdacs(int start, int count, uint8_t *values)
{
    int i, k;

    for (i = 0; i < count; i++)
        for (k = 0; k < 3; k++)
            values[3 * i + k] = s_dac[(start + i) & 0xFF][k];
    return 0;
}

/* a DOS read of up to n bytes of a file in memory: returns the bytes read */
static size_t mread(const uint8_t *data, size_t len, size_t *pos, uint8_t *dst, size_t n)
{
    size_t k = *pos < len ? len - *pos : 0;

    if (k > n)
        k = n;
    memcpy(dst, data + *pos, k);
    *pos += k;
    return k;
}

/* 0x878F7 fg_gifpal: the colour table of a GIF file, each byte >> 2. Literal, quirks included (D): no extension blocks
   are skipped before the image descriptor, and the copy always starts at the scratch buffer (0xD7694), so a file with
   both tables yields the global table with the local table's length. Returns the colour count, -1 no file, -2 bad. */
int fg_gifpal(const char *filename, uint8_t *palette)
{
    uint8_t scratch[13 + 768 + 10 + 768];
    size_t len, pos = 0, esi = 0, n3 = 0;
    uint8_t *data = load_file(filename, &len);
    int n = 0, i;

    if (data == NULL)
        return -1;                            /* 0x8790D */
    memset(scratch, 0, sizeof scratch);
    if (mread(data, len, &pos, scratch, 13) != 13 || (scratch[0] | 0x20) != 'g' || (scratch[1] | 0x20) != 'i' ||
        (scratch[2] | 0x20) != 'f')           /* 0x8792A, 0x87930 */
        goto bad;
    if (scratch[10] & 0x80) {                 /* global table: read over the start of the scratch buffer */
        n = 1 << ((scratch[10] & 7) + 1);
        n3 = (size_t)n * 3;
        if (mread(data, len, &pos, scratch, n3) == 0)
            goto bad;                         /* 0x8796A: only a 0-byte read is caught */
        esi = n3;                             /* 0x8796E add esi,ecx */
    }
    if (mread(data, len, &pos, scratch + esi, 10) == 0)   /* 0x87970: the descriptor, assumed to follow */
        goto bad;
    if (scratch[esi + 9] & 0x80) {            /* local table, read over the descriptor */
        n = 1 << ((scratch[esi + 9] & 7) + 1);
        n3 = (size_t)n * 3;
        mread(data, len, &pos, scratch + esi, n3);   /* 0x879A3: a short read is not caught */
    }
    free(data);
    if (n3 == 0)
        return 0;                             /* no table: count and length are uninitialised in the original (G) */
    for (i = 0; i < (int)n3; i++)
        palette[i] = (uint8_t)(scratch[i] >> 2);   /* 0x879C5 lodsw; shr ax,2; and al,3Fh */
    return n;
bad:
    free(data);
    return -2;                                /* 0x879A9 */
}

/* ---- GIF display (fg_showgif 0x88F98 and the catalog's decoder 0xA3D14 behave the same) ------------------- */

/* flags bit 1: upper-left at the cursor, else at the descriptor's left/top; bit 0: keep the palette, else DAC 0..n-1 =
   the colour table (local, else global) >> 2, also with a vb open. Clipped only on the right and bottom of the
   active surface; rows opaque. Returns 0, or 2 when the data is not a GIF. */
static int show_gif(const uint8_t *data, size_t len, int flags)
{
    gif_image g;
    int rc, x0, y0, w, rows, r, i;

    if (len < 3 || (data[0] | 0x20) != 'g' || (data[1] | 0x20) != 'i' || (data[2] | 0x20) != 'f')
        return 2;                             /* 0x891AA */
    rc = gif_decode(data, len, &g);
    if (rc == -3)
        return 0;                             /* ';' before any image: nothing drawn (0x893D4) */
    if (rc != 0)
        return 2;                             /* G: the original returns 2 for a bad block byte; other damage draws garbage */
    if (flags & 2) {                          /* 0x8927D */
        x0 = s_cx;
        y0 = s_cy;
    } else {
        x0 = g.left;
        y0 = g.top;
    }
    if (!(flags & 1)) {                       /* 0x89BB2 test flags,1 */
        for (i = 0; i < g.ncolors && i < 256; i++) {
            s_dac[i][0] = (uint8_t)(g.pal[i][0] >> 2);   /* 0x89A63 */
            s_dac[i][1] = (uint8_t)(g.pal[i][1] >> 2);
            s_dac[i][2] = (uint8_t)(g.pal[i][2] >> 2);
        }
        s_dirty = 1;
    }
    /* 0x892DB / 0x8932A: min(x0 + w - 1, width - 1) - x0 + 1 columns, as many rows. No test for negative x0/y0 in the
       original (it would write before the surface); interlaced rows are counted in decode order there, which only
       differs when an interlaced image is clipped (G: the game's GIFs are not, unchecked). gif.c returns the rows
       in display order. */
    w = (x0 + g.w - 1 < s_sw - 1 ? x0 + g.w - 1 : s_sw - 1) - x0 + 1;
    rows = (y0 + g.h - 1 < s_sh - 1 ? y0 + g.h - 1 : s_sh - 1) - y0 + 1;
    if (w > 0 && rows > 0) {
        int64_t size = surf_size();
        for (r = 0; r < rows; r++)
            copy_fwd(s_surf, size, ((int64_t)y0 + r) * s_sw + x0, g.pixels, (int64_t)g.w * g.h, (int64_t)r * g.w, w);
        touch();
    }
    gif_free(&g);
    return 0;
}

/* 0x88F98 fg_showgif: from the file, or (flags & 4) from the fg_imagebuf buffer. 1 when the file cannot be opened. */
int fg_showgif(const char *filename, int flags)
{
    size_t len;
    uint8_t *data;
    int rc;

    if (flags & 4)                            /* 0x88FE8 */
        return show_gif(s_imgbuf, s_imgsize, flags);
    data = load_file(filename, &len);
    if (data == NULL)
        return 1;                             /* 0x88FFD */
    rc = show_gif(data, len, flags);
    free(data);
    return rc;
}

/* 0x8240F fg_imagebuf */
int fg_imagebuf(uint8_t *buffer, uint32_t size)
{
    if (size == 0) {                          /* 0x8242E: the internal buffer */
        s_imgbuf = s_imgbuf_internal;
        s_imgsize = sizeof s_imgbuf_internal;
    } else {
        s_imgbuf = buffer;
        s_imgsize = size;
    }
    return 0;
}

/* ---- FLI/FLC playback (0x92451 fg_flicsize, 0x92486 fg_showflic) ------------------------------------------------- */

/* 0x92451 fg_flicsize: the FLI/FLC header's width/height; both -1 for any other magic. Always returns 0. */
int fg_flicsize(const uint8_t *header, int *width, int *height)
{
    uint32_t magic = (uint32_t)header[4] | ((uint32_t)header[5] << 8);

    if (magic == 0xAF11u || magic == 0xAF12u) {   /* 0x92458 */
        *width = (int)((uint32_t)header[8] | ((uint32_t)header[9] << 8));    /* 0x92451: movzx [esi+8], [esi+0xA] */
        *height = (int)((uint32_t)header[0xA] | ((uint32_t)header[0xB] << 8));
    } else {
        *width = -1;                          /* 0x9247F */
        *height = -1;
    }
    return 0;                                 /* 0x9247A xor eax,eax */
}

/* header bytes of a possibly short file: missing bytes read as 0 (G: the original does not detect a short read) */
static unsigned flic_hb(const uint8_t *d, size_t size, size_t off)
{
    return off < size ? d[off] : 0;
}

static uint32_t flic_hw(const uint8_t *d, size_t size, size_t off)
{
    return (uint32_t)flic_hb(d, size, off) | ((uint32_t)flic_hb(d, size, off + 1) << 8);
}

static uint32_t flic_hd(const uint8_t *d, size_t size, size_t off)
{
    return flic_hw(d, size, off) | (flic_hw(d, size, off + 2) << 16);
}

/* the byte reader (0x925EB..0x92605): left = bytes available; reading at left == 0 returns -1 and does not
   advance; every read also decrements the current chunk's counter crem */
typedef struct {
    const uint8_t *p;
    int32_t left;
    int32_t crem;
} flrd_t;

static int32_t flic_rd(flrd_t *r)
{
    if (r->left == 0)
        return -1;
    r->left--;
    r->crem--;
    return *r->p++;
}

/* words are assembled as the original does (mov bl,al / mov ah,al / mov al,bl on eax): w is 0xFFFFFFFF only when
   the second read ran dry and the first byte was 0xFF (or dry); a real 0xFFFF word gives 0x0000FFFF */
static uint32_t flic_word(flrd_t *r)
{
    int32_t b1 = flic_rd(r), b2 = flic_rd(r);
    uint32_t w = (uint32_t)b2;

    return (w & 0xFFFF0000u) | (((uint32_t)b2 & 0xFFu) << 8) | ((uint32_t)b1 & 0xFFu);
}

/* dwords: low word then high word, each assembled the same way, then low | high << 16 of their low 16 bits */
static uint32_t flic_dword(flrd_t *r)
{
    uint32_t lo = flic_word(r), hi = flic_word(r);

    return (lo & 0xFFFFu) | ((hi & 0xFFFFu) << 16);
}

static void flic_skip(flrd_t *r, uint32_t n)
{
    while (n-- > 0)
        (void)flic_rd(r);                     /* if fewer are left, left becomes 0 */
}

/* a run of n bytes at (x, y) goes to offset y*640 + x onward, so a run past x 639 continues on the next scanline;
   the only clip is y >= 480 (signed compare, D, 0x9325A). Output always goes to the screen, even with a virtual
   buffer open (the original's table 0x93375 has no vb entry; G: it would crash). The port also drops bytes outside
   the screen (negative offsets, past the end: G, the original writes outside there). */
static void flic_write(int x, int y, const uint8_t *src, uint32_t n)
{
    int64_t off;
    uint32_t i;

    if (y >= SCR_H)
        return;
    off = (int64_t)y * SCR_W + x;
    for (i = 0; i < n; i++)
        if (off + (int64_t)i >= 0 && off + (int64_t)i < (int64_t)SCR_W * SCR_H)
            s_screen[off + (int64_t)i] = src[i];
}

/* 0x92486's palette chunks (types 4 COLOR_256 and 11 COLOR_64, both skipped when flags & 0x10): packet count word,
   then ONE retrace wait for the whole chunk (correction 1), index = 0, per packet: index += skip byte (8-bit wrap),
   count byte (0 = 256), count RGB triples into s_dac (type 4: byte >> 2; type 11: byte & 0x3F, G: the original writes
   the byte unchanged to the 6-bit DAC), index += 1 per triple (8-bit wrap). A packet count of 0 loops ~2^32 in the
   original (correction 5); the port treats it as 0 packets (G). */
static void flic_color(flrd_t *r, uint32_t ctype)
{
    uint32_t packets = flic_word(r);
    uint8_t index = 0;

    if (packets == 0)
        return;
    pace_retrace();                           /* one wait per chunk, before the first packet */
    while (packets-- > 0) {
        uint32_t skip = (uint32_t)flic_rd(r) & 0xFFu;
        uint32_t cnt = (uint32_t)flic_rd(r) & 0xFFu;
        uint32_t j;
        int k;

        if (cnt == 0)
            cnt = 256;
        index = (uint8_t)(index + skip);
        for (j = 0; j < cnt; j++) {
            for (k = 0; k < 3; k++) {
                uint32_t b = (uint32_t)flic_rd(r) & 0xFFu;

                s_dac[index][k] = ctype == 4 ? (uint8_t)(b >> 2) : (uint8_t)(b & 0x3Fu);
            }
            index = (uint8_t)(index + 1);
        }
    }
    s_dirty = 1;
}

/* type 15 BRUN: per row the packet-count byte (the loop bound, as in the verified flc.c), then until W bytes are
   done: signed n, n > 0 replicates the next byte n times, n < 0 copies -n literal bytes (n == 0: nothing, G) */
static void flic_brun(flrd_t *r, int x0, int y0, uint32_t W, uint32_t H, uint8_t *row)
{
    uint32_t y;

    for (y = 0; y < H; y++) {                /* H = 0: the original loops ~2^32 rows; the port draws nothing (G) */
        uint32_t packets = (uint32_t)flic_rd(r) & 0xFFu;
        uint32_t x = 0;

        while (packets-- > 0 && x < W) {
            int c = (int8_t)((uint32_t)flic_rd(r) & 0xFFu);

            if (c > 0) {
                uint8_t v = (uint8_t)((uint32_t)flic_rd(r) & 0xFFu);
                memset(row + x, v, (size_t)c);
                flic_write(x0 + (int)x, y0 + (int)y, row + x, (uint32_t)c);
                x += (uint32_t)c;
            } else if (c < 0) {
                uint32_t n = (uint32_t)(-c), i;
                for (i = 0; i < n; i++)
                    row[x + i] = (uint8_t)((uint32_t)flic_rd(r) & 0xFFu);
                flic_write(x0 + (int)x, y0 + (int)y, row + x, n);
                x += n;
            }
        }
    }
}

/* type 16 COPY: H rows of W literal bytes, each row padded to (W+3)&~3 bytes in the file (as the verified flc.c
   reads them) */
static void flic_copy(flrd_t *r, int x0, int y0, uint32_t W, uint32_t H, uint8_t *row)
{
    uint32_t rowb = (W + 3u) & ~3u;
    uint32_t y;

    for (y = 0; y < H; y++) {
        uint32_t i;

        for (i = 0; i < W; i++)
            row[i] = (uint8_t)((uint32_t)flic_rd(r) & 0xFFu);
        flic_write(x0, y0 + (int)y, row, W);
        flic_skip(r, rowb - W);
    }
}

/* type 13 BLACK: H rows of W zeros at (x0, y0 + r), each row with the same clip rule (G: the flicplay entry's stale-y
   quirk is not checked for showflic) */
static void flic_black(int x0, int y0, uint32_t W, uint32_t H, uint8_t *row)
{
    uint32_t y;

    memset(row, 0, W);
    for (y = 0; y < H; y++)
        flic_write(x0, y0 + (int)y, row, W);
}

/* type 12 LC (DELTA_FLI): y = y0 + word skip; word line count; per line: unsigned packet-count byte; x = 0; per
   packet: x += unsigned skip byte; signed n: n >= 0 copies n literal bytes, n < 0 replicates the next byte -n times */
static void flic_lc(flrd_t *r, int x0, int y0, uint8_t *row)
{
    uint32_t start = flic_word(r) & 0xFFFFu;
    uint32_t lines = flic_word(r) & 0xFFFFu;
    int y = y0 + (int)start;
    uint32_t l;

    for (l = 0; l < lines; l++) {
        uint32_t packets = (uint32_t)flic_rd(r) & 0xFFu;
        uint32_t x = 0;

        while (packets-- > 0) {
            uint32_t skip = (uint32_t)flic_rd(r) & 0xFFu;
            int c = (int8_t)((uint32_t)flic_rd(r) & 0xFFu);

            x += skip;
            if (c >= 0) {
                uint32_t i, n = (uint32_t)c;
                for (i = 0; i < n; i++)
                    row[i] = (uint8_t)((uint32_t)flic_rd(r) & 0xFFu);
                flic_write(x0 + (int)x, y, row, n);
                x += n;
            } else {
                uint32_t n = (uint32_t)(-c);
                uint8_t v = (uint8_t)((uint32_t)flic_rd(r) & 0xFFu);
                memset(row, v, n);
                flic_write(x0 + (int)x, y, row, n);
                x += n;
            }
        }
        y++;
    }
}

/* type 7 SS2 (DELTA_FLC): y = y0; word line count; per line read words until a packet count: bit15+bit14 set ->
   y += -word (lines skipped); bit15 set only -> the low byte goes to (xr, y); else the packet count (0 goes to the
   next line). Per packet: x += unsigned skip byte; signed n: n < 0 replicates the next 2-byte pair -n times,
   n > 0 copies n literal 16-bit pairs (2n bytes); written at x, x += 2*|n| */
static void flic_ss2(flrd_t *r, int x0, int y0, int xr, uint8_t *row)
{
    uint32_t lines = flic_word(r) & 0xFFFFu;
    int y = y0;

    while (lines > 0) {
        int lp = (int16_t)(flic_word(r) & 0xFFFFu);

        if ((lp & 0xC000) == 0xC000) {
            y += -lp;
        } else if ((lp & 0xC000) == 0x8000) {
            uint8_t b = (uint8_t)(lp & 0xFF);
            flic_write(xr, y, &b, 1);         /* the low byte at the right edge (xr, y) */
        } else {
            uint32_t packets = (uint32_t)lp;
            uint32_t x = 0;

            lines--;
            while (packets-- > 0) {
                uint32_t skip = (uint32_t)flic_rd(r) & 0xFFu;
                int c = (int8_t)((uint32_t)flic_rd(r) & 0xFFu);

                x += skip;
                if (c < 0) {
                    uint32_t n = (uint32_t)(-c), i;
                    uint8_t b1 = (uint8_t)((uint32_t)flic_rd(r) & 0xFFu);
                    uint8_t b2 = (uint8_t)((uint32_t)flic_rd(r) & 0xFFu);

                    row[0] = b1;
                    row[1] = b2;
                    for (i = 0; i < n; i++)
                        flic_write(x0 + (int)x + 2 * (int)i, y, row, 2);
                    x += 2 * n;
                } else if (c > 0) {
                    uint32_t n = (uint32_t)c * 2u, i;
                    for (i = 0; i < n; i++)
                        row[i] = (uint8_t)((uint32_t)flic_rd(r) & 0xFFu);
                    flic_write(x0 + (int)x, y, row, n);
                    x += n;
                }
            }
            y++;
        }
    }
}

/* 0x92486 fg_showflic(filename, count, flags): play a FLI/FLC. flags bit 0 = no frame delay, bit 1 = top-left at the
   graphics cursor (else (0,0)), bit 2 = the data is fg_imagebuf's buffer (the 128-byte header at its start, data from
   +0x80; filename ignored), bit 4 = ignore palette chunks. Returns 0 when played or stopped, 1 when the file cannot
   be opened, 2 for a bad magic. */
int fg_showflic(const char *filename, int count, int flags)
{
    const uint8_t *data;
    uint8_t *filebuf = NULL;
    size_t size;
    uint32_t frames, W, H, delay, magic;
    int x0, y0, xr;
    flrd_t r;
    int32_t fleft;
    uint8_t *row;
    int rc = 0;

    /* the colour count check: the port is always in mode 25 (256 colours, fg_setmode at 0x109F3), so the original's
       [0xD9F8C] != 0x100 early return (0x9249A) never happens here */
    if (flags & 4) {
        data = s_imgbuf;
        size = s_imgsize;
    } else {
        filebuf = load_file(filename, &size);
        if (filebuf == NULL)
            return 1;                         /* 0x924E4 */
        data = filebuf;                       /* the port reads the whole file and then behaves as memory mode with
                                                 size = the file length (G: the original streams it through the
                                                 imagebuf; the bytes and the end-of-data rule are the same) */
    }
    magic = flic_hw(data, size, 4);
    if (magic != 0xAF11u && magic != 0xAF12u) {   /* 0x9250D */
        free(filebuf);
        return 2;
    }
    frames = flic_hw(data, size, 6);
    W = flic_hw(data, size, 8);
    H = flic_hw(data, size, 0xA);
    if (magic == 0xAF11u) {
        uint32_t speed16 = flic_hw(data, size, 0x10);

        delay = (uint16_t)((speed16 * 1429u) / 5500u);   /* 0x9252C: the 32-bit dx:ax product, the 16-bit div of
                                                             dx:ax by 5500 (a quotient > 0xFFFF would fault; cannot
                                                             happen for a 16-bit speed) */
    } else {
        uint32_t speed32 = flic_hd(data, size, 0x10);

        delay = (uint16_t)(speed32 / 55u);    /* 0x92544: a quotient > 0xFFFF faults in DOS; the port takes it
                                                 truncated to 16 bits (G) */
    }
    if (flags & 2) {                          /* 0x92598 */
        x0 = fg_getxpos();
        y0 = fg_getypos();
    } else {
        x0 = 0;
        y0 = 0;
    }
    xr = x0 + (int)W - 1;                     /* 0x925BC: SS2's "last byte" column */

    row = malloc((size_t)W + 256u);           /* the decoders' scratch row */
    if (row == NULL) {
        free(filebuf);
        return 0;
    }
    r.p = data + 0x80;                        /* 0x925D6 */
    r.left = size >= 0x80 ? (int32_t)(size - 0x80) : 0;
    r.crem = 0;
    fleft = (int32_t)frames;                  /* 0x9255F; frames 0 wraps to an endless pass */

    for (;;) {
        uint32_t szlo = flic_word(&r);

        if (szlo == 0xFFFFFFFFu)              /* 0x9262A -> 0x930CC: end of pass */
            goto end_pass;
        {
            uint32_t szhi = flic_word(&r);
            uint32_t size = (szlo & 0xFFFFu) | ((szhi & 0xFFFFu) << 16);
            uint32_t type = flic_word(&r);
            uint32_t slack;

            if (type == 0xF100u) {            /* prefix: skipped, but counts as a frame (correction 6) */
                slack = size - 6;             /* 0x9281E */
                goto key_check;
            }
            if (type != 0xF1FAu) {            /* 0x926B4 -> 0x93127: stop, return 0 */
                rc = 0;
                goto done;
            }
            slack = size - 16u;               /* 0x926BE, 16-bit sub/sbb: 32-bit */
            {
                uint32_t nchunks = flic_word(&r);   /* 0x9270A */
                uint32_t c;

                flic_skip(&r, 8);             /* 0x9270D the reserved bytes */
                for (c = 0; c < nchunks; c++) {
                    uint32_t csize = flic_dword(&r);
                    uint32_t ctype;

                    slack -= csize;           /* 0x927B1 */
                    r.crem = (int32_t)(csize - 4u);   /* 0x927C1 */
                    ctype = flic_word(&r);
                    switch (ctype) {
                    case 4:
                    case 11:
                        if ((flags & 0x10u) == 0)
                            flic_color(&r, ctype);
                        break;
                    case 7:
                        flic_ss2(&r, x0, y0, xr, row);
                        break;
                    case 12:
                        flic_lc(&r, x0, y0, row);
                        break;
                    case 13:
                        flic_black(x0, y0, W, H, row);
                        break;
                    case 15:
                        flic_brun(&r, x0, y0, W, H, row);
                        break;
                    case 16:
                        flic_copy(&r, x0, y0, W, H, row);
                        break;
                    default:                  /* every other type, including > 0x10, is skipped (correction 3) */
                        break;
                    }
                    if (r.crem < 0)           /* 0x93041: the handler overran the chunk -> end of pass */
                        goto end_pass;
                    flic_skip(&r, (uint32_t)r.crem);
                }
            }
        key_check:                            /* 0x9306B */
            {
                unsigned char k, a;

                fg_intkey(&k, &a);
                if (k == 27) {                /* 0x93079 -> 0x9311C: Esc stops */
                    rc = 0;
                    goto done;
                }
            }
            flic_skip(&r, slack);             /* 0x9307E */
            if ((flags & 1u) == 0 && delay != 0)
                fg_waitfor((int)delay);       /* 0x93086 */
            touch();                          /* the screen may have changed */
            fleft--;                          /* 0x930C2 */
            if (fleft != 0)
                continue;
        }
    end_pass:                                 /* 0x930CC */
        if (count != 0) {
            count--;
            if (count == 0) {
                rc = 0;
                goto done;
            }
        }
        fleft = (int32_t)frames;
        r.p = data + 0x80;
        r.left = size >= 0x80 ? (int32_t)(size - 0x80) : 0;
        r.crem = 0;                           /* 0x93104: rewind, next pass */
    }
done:
    free(row);
    free(filebuf);                            /* file mode frees its buffer at every return */
    return rc;
}

/* ---- catalogs (0xA22C3..0xA25B3) -------------------------------------------------------------------------------- */

#define CAT_HANDLE0 5                         /* handles as DOS gives them after its five standard ones (G) */

static fg_cat *cat_get(int handle)
{
    int i = handle - CAT_HANDLE0;

    return i >= 0 && i < NCAT && s_cat[i].used ? &s_cat[i] : NULL;
}

/* 0xA22C3: the file and its directory; the file pointer ends after the first block (0x318) */
int cat_open_file(const char *path)
{
    int i;

    for (i = 0; i < NCAT; i++)
        if (!s_cat[i].used)
            break;
    if (i == NCAT)
        return 0;
    s_cat[i].data = load_file(path, &s_cat[i].size);
    if (s_cat[i].data == NULL)
        return 0;
    if (cat_open(&s_cat[i].cat, s_cat[i].data, s_cat[i].size) != 0) {   /* G: the original reads the chain lazily */
        free(s_cat[i].data);
        s_cat[i].data = NULL;
        return 0;
    }
    s_cat[i].used = 1;
    s_cat[i].pos = 0x318;
    return CAT_HANDLE0 + i;
}

/* the shared lookup (0xA2309-0xA2385): name copied into a zeroed 12-byte key with 'a'..'z' upper-cased (a longer name
   is cut to 12 bytes for the compare), then all 12 bytes compared with each entry in directory order */
static const cat_entry *cat_lookup(const fg_cat *c, const char *name)
{
    char key[12];
    int i;

    memset(key, 0, sizeof key);
    for (i = 0; i < 12 && name[i] != 0; i++)
        key[i] = (char)(name[i] >= 'a' && name[i] <= 'z' ? name[i] - 0x20 : name[i]);
    for (i = 0; i < c->cat.count; i++)
        if (memcmp(c->cat.entries[i].name, key, 12) == 0)   /* rep cmpsw x6 */
            return &c->cat.entries[i];
    return NULL;
}

/* 0xA22FF cat_show: 0 ok, -1 DOS error (here a bad handle), -2 not found, -3 decoder failed or a type it cannot show */
int cat_show(const char *name, int flags, int handle)
{
    fg_cat *c = cat_get(handle);
    const cat_entry *e;
    int rc;

    if (c == NULL)
        return -1;
    e = cat_lookup(c, name);
    if (e == NULL)
        return -2;
    flags &= 0xFFFB;                          /* 0xA2400 and dword,0x0000FFFB: always from the file */
    c->pos = (int64_t)e->offset + e->length;  /* G: where the decoder leaves the file pointer */
    switch (e->kind & 0xFFu) {                /* 0xA2412 mov al,[esi+0x14]: the type BYTE (+0x16 is a separate word) */
    case 2:                                   /* GIF, 0xA3D14 */
        if (cat_data(&c->cat, e) == NULL)
            return -3;
        rc = show_gif(cat_data(&c->cat, e), e->length, flags);
        return rc != 0 ? -3 : 0;
    case 3:
    case 4:                                   /* 0xA5936, not decoded; no such entries in the catalogs (G) */
        return 0;
    default:                                  /* 1 PCX, 5/6 FLI/FLC (0xA2D29, not decoded): none reaches here, the game's
                                                 11 calls all show GIFs with flags 3 (G); other types -3 (D) */
        return -3;
    }
}

/* 0xA24A7 cat_size: the entry's length; the file pointer at its data */
long cat_size(const char *name, int handle)
{
    fg_cat *c = cat_get(handle);
    const cat_entry *e;

    if (c == NULL)
        return -1;
    e = cat_lookup(c, name);
    if (e == NULL)
        return -2;
    c->pos = e->offset;                       /* 0xA2587 */
    return (long)e->length;                   /* 0xA259C */
}

/* 0xA25B3 cat_load: the entry's bytes into buffer */
int cat_load(const char *name, uint8_t *buffer, int handle)
{
    fg_cat *c = cat_get(handle);
    const cat_entry *e;
    const uint8_t *d;

    if (c == NULL)
        return -1;
    e = cat_lookup(c, name);
    if (e == NULL)
        return -2;
    d = cat_data(&c->cat, e);
    if (d == NULL)
        return -3;
    memcpy(buffer, d, e->length);             /* 0xA26B5 one read of size bytes */
    c->pos = (int64_t)e->offset + e->length;
    return 0;
}

/* _dos_read (0x92420) on a catalog handle: INT 21h AH=3Fh from the file pointer */
int cat_read(int handle, uint8_t *buffer, unsigned count, unsigned *bytes)
{
    fg_cat *c = cat_get(handle);
    size_t pos;

    if (c == NULL)
        return 6;                             /* DOS error 6, invalid handle; *bytes not written */
    pos = c->pos < 0 ? 0 : (size_t)c->pos;
    *bytes = (unsigned)mread(c->data, c->size, &pos, buffer, count);
    c->pos = (int64_t)pos;
    return 0;
}

/* ---- fonts (Fastgraph/Fonts; glyphs through fgfont.c) ---------------------------------------------------------- */

/* 0x8DFEE fgf_define: the first free of 32 slots; the new font becomes the current one; returns 1..32, 0 when full.
   The original turns the caller's glyphs bottom row first in place; fgfont.c reads them as stored. */
int fgf_define(const uint8_t *data)
{
    int i;

    for (i = 0; i < 32; i++)                  /* 0x8E005 */
        if (!s_font_used[i])
            break;
    if (i == 32)
        return 0;                             /* 0x8E00D */
    if (fgfont_load(data, (size_t)194 + (size_t)96 * data[0] * data[1], &s_font[i]) != 0)
        return 0;                             /* G: the original defines any bytes */
    s_font_used[i] = 1;
    s_font_cur = i + 1;                       /* 0x8E016 [0xE0A74] = slot, 0x8E0D7 baseline */
    return i + 1;
}

/* 0x8E188 fgf_select: 1..32 and defined, else nothing */
int fgf_select(int font)
{
    if (font >= 1 && font <= 32 && s_font_used[font - 1])
        s_font_cur = font;
    return 0;
}

/* 0x8E1EB: the most rows of the string's glyphs; 0 for n <= 0 or no font */
int fgf_height(const char *s, int n)
{
    if (n <= 0 || s_font_cur == 0)
        return 0;
    return fgfont_height(&s_font[s_font_cur - 1], s, n);
}

/* 0x8E23E: [0xE0A6C], 0 before any font is defined */
int fgf_baseline(void)
{
    return s_font_cur != 0 ? s_font[s_font_cur - 1].baseline : 0;
}

/* 0x8E244: [0xE0A70] */
int fgf_spacing(int pixels)
{
    s_spacing = pixels;
    return 0;
}

/* 0x8E253 fgf_print: at the cursor (its y is the baseline), current colour, glyphs through fg_drawmap; 0x7F + c sets
   the colour through fg_setcolor (it stays set afterwards). The cursor ends at (x after the text, the original y)
   (0x8E283 pushes y; 0x8E42F fg_move). Justification (0xE0A80/81) stays off: the game never sets it (D, both 0xFF in
   the data). Differences of fgfont.c (G, none reached by the game's texts): it clips glyphs at the surface where
   fg_drawmap would wrap linearly, and a 0x7F as the last byte ends the string where the original reads the byte after
   it in its copy buffer. */
int fgf_print(const char *s, int n)
{
    int x = s_cx;
    uint8_t col = s_color;

    if (n <= 0 || s_font_cur == 0)            /* 0x8E261 */
        return 0;
    fgfont_print(s_surf, s_sw, s_sh, &s_font[s_font_cur - 1], &x, s_cy, s, n, s_spacing, &col);
    s_color = col;
    fg_move(x, s_cy);
    touch();
    return 0;
}

/* 0x8A7DD: the advance of n bytes, spacing after every glyph; 0 for n <= 0 or no font */
int fgf_width(const char *s, int n)
{
    if (n <= 0 || s_font_cur == 0)
        return 0;
    return fgfont_width(&s_font[s_font_cur - 1], s, n, s_spacing);
}

/* ---- mouse and keyboard ---------------------------------------------------------------------------------------- */

/* 0x814BD fg_mouseini (mode 25 path 0x81762): the pointer starts hidden ([0xD5FD7] = 0, 0x818D2). The original also
   centres the mouse (INT 33h AX=4); the port cannot move the player's pointer and takes where it is. */
int fg_mouseini(void)
{
    int b;

    s_mouse_init = 1;
    s_mouse_vis = 0;
    s_sys_pointer = g_ds == NULL;             /* no data object: no masks, the system pointer stands in */
    plat_mouse(&s_mx, &s_my, &b);
    s_mb = 0;
    s_mlatch = 0;
    plat_pointer(0);
    return 2;
}

/* 0x91E9E fg_mousevis: nothing unless the mouse is initialised and state & 1 differs from [0xD5FD7]; then
   [0xD5FD7] = state. The port draws the pointer at present time (fg_present). */
int fg_mousevis(int state)
{
    if (!s_mouse_init || (state & 1) == s_mouse_vis)   /* 0x91EA6, 0x91EB5 */
        return 0;
    s_mouse_vis = state & 1;                  /* 0x92360: al = state & 1 (pushed at 0x91EEC) */
    if (s_sys_pointer)
        plat_pointer(state & 1);
    return 0;
}

/* 0x8C22C fg_mousepos: position in pixels, buttons bit 0 left, 1 right, 2 middle; zeros without fg_mouseini.
   x one less when it reaches the surface width (0x8C28C). */
void fg_mousepos(int *x, int *y, int *buttons)
{
    service();
    if (!s_mouse_init) {                      /* 0x8C2AB */
        *x = *y = *buttons = 0;
        return;
    }
    *x = s_mx >= s_sw ? s_mx - 1 : s_mx;
    *y = s_my;
    *buttons = s_mb | s_mlatch;
    s_mlatch = 0;
}

/* 0x87E43 fg_mousefin: unhooks the handler; nothing visible changes (it neither hides the pointer nor clears 0xD5FD6) */
void fg_mousefin(void)
{
}

/* 0x87E7C fg_intkey: INT 16h AH=01h / 00h; an ASCII key gives (ascii, 0), an extended one (0, scan) */
void fg_intkey(unsigned char *key, unsigned char *aux)
{
    uint16_t k;

    service();
    if (s_headless && s_test_key != 0) {     /* test hook: fg_test_key */
        *key = (unsigned char)s_test_key;
        *aux = 0;
        s_test_key = 0;
        return;
    }
    if (s_nkeys == 0) {
        *key = 0;
        *aux = 0;
        return;
    }
    k = s_keys[0];
    memmove(s_keys, s_keys + 1, (size_t)(s_nkeys - 1) * sizeof s_keys[0]);
    s_nkeys--;
    if ((k & 0xFF) != 0) {                    /* 0x87E92 or al,al */
        *key = (unsigned char)(k & 0xFF);
        *aux = 0;
    } else {
        *key = 0;
        *aux = (unsigned char)(k >> 8);
    }
}

/* Fastgraph's fg_waitfor: ticks of the 18.2 Hz BIOS clock (no address identified in LIBRARY_CALLS.md; G). Headless:
   no wait. */
void fg_waitfor(int ticks)
{
    uint64_t end;

    service();
    if (ticks <= 0 || s_headless)
        return;
    end = plat_ticks_ms() + (uint64_t)ticks * 10000u / 182u;
    while (plat_ticks_ms() < end) {
        plat_sleep_ms(1);
        service();
    }
}
