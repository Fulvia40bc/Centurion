#ifndef NAVAL_INT_H
#define NAVAL_INT_H
/* The naval battle's internals, shared by naval.c (logic, set-up, the duel loop, the fleet battle) and naval_gfx.c (the
 * EXE drawing helpers it calls and section 5's drawing routines). Addresses are 0823 offsets (OVL_NAVAL.asm / EXE).
 * Side s: 0 = the enemy (computer) flagship, 1 = the Roman (human) flagship; per-side DS arrays are base + s (bytes),
 * base + 2s (words), base + 4s (x, y pairs), as in docs/NAVAL_LOGIC_NOTES.md section 2. */
#include <stdint.h>

/* battle state block (NAVAL_LOGIC_NOTES 2) */
#define COUR(s)   (0xC36E + (s))        /* b  courage */
#define BEAR(s)   (0xC370 + (s))        /* b  bearing to the other ship (C371 = C641, C370 = C371 - 8) */
#define PX(s)     (0xC372 + 4 * (s))    /* w  x */
#define PY(s)     (0xC374 + 4 * (s))    /* w  y */
#define TYPE(s)   (0xC37A + (s))        /* b  flagship type 0..2 */
#define SPEEDMSG  0xC37C                /* b  frames left of the speed word */
#define CREW(s)   (0xC37E + 2 * (s))    /* w  men */
#define DIST      0xC382                /* w  distance */
#define RAMMER    0xC384                /* w  ship drawn in front / the rammer */
#define ROUNDT    0xC386                /* w  ticks to the next fleet round / melee round */
#define HEAD(s)   (0xC388 + (s))        /* b  heading 0..15 */
#define GEAR(s)   (0xC38A + (s))        /* b  speed index */
#define OARS(s)   (0xC38C + (s))        /* b  oar level 0..4 */
#define HULL(s)   (0xC38E + (s))        /* sb flagship strength, 0 = sunk */
#define OARF(s)   (0xC390 + (s))        /* b  oar frame 0..6 */
#define POT(s)    (0xC392 + (s))        /* b  fire pot: 0, FF flying, 1 splash */
#define OARREQ(s) (0xC394 + (s))        /* sb oar level request */
#define TURNREQ(s)(0xC396 + (s))        /* sb turn request */
#define FX(s)     (0xC398 + 4 * (s))    /* w  fraction of x */
#define FY(s)     (0xC39A + 4 * (s))    /* w  fraction of y */
#define FIRE(s)   (0xC3A0 + (s))        /* b  burn counter */
#define SHEET(s)  (0xC3A2 + 2 * (s))    /* w  ship sheet handle */
#define H_OARS1   0xC3A6
#define H_OARS2   0xC3A8
#define HOOKS(s)  (0xC3AA + (s))        /* b  grappling hooks / corvus frames left */
#define H_SMOK    0xC3AC
#define H_BOMBS   0xC3AE
#define ARROW(s)  (0xC3BA + (s))        /* b  arrow volley flying */
#define H_FLAG    0xC3BC
#define REM(t)    (0xC3BE + 2 * (t))    /* w  remainders of the Roman losses */
#define ARANGE(s) (0xC3C4 + 2 * (s))    /* w  arrow range left */
#define PRANGE(s) (0xC3C8 + 2 * (s))    /* w  pot range left */
#define PDIR(s)   (0xC3CC + (s))        /* b */
#define ADIR(s)   (0xC3CE + (s))        /* b */
#define POTX(s)   (0xC3D0 + 4 * (s))
#define POTY(s)   (0xC3D2 + 4 * (s))
#define ARRX(s)   (0xC3D8 + 4 * (s))
#define ARRY(s)   (0xC3DA + 4 * (s))
#define POTFX(s)  (0xC3E0 + 4 * (s))
#define POTFY(s)  (0xC3E2 + 4 * (s))
#define ARRFX(s)  (0xC3E8 + 4 * (s))
#define ARRFY(s)  (0xC3EA + 4 * (s))
#define RECT(s)   (0xC3F0 + 8 * (s))    /* 4w ship sprite rect x0, y0, x1, y1 */
#define POTANIM(s)(0x98BE + (s))        /* b */
#define FIREANIM(s)(0x98E0 + (s))       /* b */
#define SHARKF(k) (0xAAC6 + (k))        /* b  shark frame (k 0 = the Roman wreck's, 1 = the enemy's) */
#define SHARKX(k) (0xAAC8 + 2 * (k))
#define SHARKY(k) (0xAACC + 2 * (k))
#define DIFF      B(0x0491)             /* naval difficulty 0..3 */

/* naval.c */
extern int nv_burn_on;                  /* 1990 bug 9.1 fix: the flames burn the hulls only while the duel runs */
void     nv_t_start_00D7(int n);
void     nv_t_reset_00F8(void);
void     nv_t_all_00FF(void);
uint16_t nv_t_since_0183(int n);
uint16_t nv_t_since0_01AE(void);
void     nv_wait_011F(uint16_t n);
void     nv_tick(void);                 /* one 72.8 Hz tick passes (the port's clock, see naval.c) */
int      nv_d070(int x, int y, int s);  /* D070: point in ship s */
uint16_t nv_rng(uint16_t n);            /* 076B */

/* naval_gfx.c: EXE helpers */
extern uint8_t *nv_fb;                  /* the current target [C0D0]: g_scr or g_wrk */
int      nv_vga(void);                  /* [9B26] == 2: always in the port */
void     nv_target_work_67E4(void);
void     nv_target_screen_680F(void);
void     nv_page_6837(void);
void     nv_flip_6887(void);
void     nv_save_67B6(void);
void     nv_save_691B(int x0, int y0, int x1, int y1);
void     nv_restore_6973(int x0, int y0, int x1, int y1);
void     nv_fill_7E74(int x0, int y0, int x1, int y1);
void     nv_hline_7ECA(int x0, int y, int x1);
void     nv_font(uint16_t block);       /* rep movsw DS:9B56 <- block (A05C big, A3B4 small) */
void     nv_clip(uint16_t y1);          /* C0E0 = C1EA = 9AC6 = y1 */
void     nv_text_8BE4(const char *s, int cx, int y);
void     nv_num_8A9E(uint32_t v, int x, int y);
void     nv_num_8AC8(uint32_t v, int x, int y);
void     nv_num_8AF2(uint32_t v, int x, int y, int w);
void     nv_num_8B2C(uint32_t v, int x, int y, int w);
int      nv_rect_817C(int16_t *out, const int16_t *a, const int16_t *b);   /* out NULL = DS:9AE8 */
void     nv_rect_81EA(int16_t *out, const int16_t *a, const int16_t *b);
int      nv_inrect_8238(int x, int y, const int16_t *r);
int      nv_archive_0F7B(int n);        /* 8 MNAVAL, 9 NAVL; 1 = open */
int      nv_galleys_0F0C(void);         /* GALLEYS.LZW into the work buffer */
int      nv_collide_11C4(int8_t *dst, int n);   /* COLLIDE.LZW (816 bytes) */
uint16_t nv_sheet_163B(const char *name);       /* a handle (1..) or 0 */
void     nv_sheet_19A0(uint16_t h);
void     nv_sprite_8E62(uint16_t h, int frame, int x, int y);
int      nv_desc(uint16_t h, int frame, int *w, int *xhot, int *yhot, int *rows);   /* 0 = no such frame */

/* naval_gfx.c: section 5 drawing */
void     nv_ship_B8E3(int s);
void     nv_ships_B96A(void);
uint16_t nv_hooks_BA7A(int s);
void     nv_shots_CCDE(int s);
void     nv_fire_CD4B(int s);
void     nv_panel_C71B(void);
void     nv_totals_DC8E(uint8_t e, uint8_t r);
void     nv_screen_BEAE(uint8_t e, uint8_t r);
void     nv_intro_C105(void);
void     nv_palette_restore(void);      /* the fix of the dead C18C: the cycled colours back to CMAP.BIN */
void     nv_sink_D3F2(uint16_t bits);
void     nv_sink_D60C(uint16_t bits);

#endif
