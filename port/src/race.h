#ifndef RACE_H
#define RACE_H
/* The chariot race and the victory sequence, OVL0 section 4 (file 0x51A0-0x8550, loaded at 0823:B0E0); disasm/OVL_RACE.asm,
 * docs/RACE_INDEX.md, RACE_LOGIC_NOTES.md, RACE_SCREEN_NOTES.md, RACE_DATA_NOTES.md. Logic, timing and dice in race.c, the
 * drawing in race_gfx.c (on top of naval_gfx.c's EXE helpers: targets, fills, text, the sprite-sheet loader and blitter). */
#include <stdint.h>

/* section 4 D08A via thunk B0AA (Rome Games "Racing Chariots", EXE 0x349D): the whole race; no arguments, no result */
void race_d08a(void);
/* section 4 DF58 via thunk B092 (the map loop at rank Caesar, EXE 0x09A4): statistics, epilogue, triumph, credits.
   The caller ends the game afterwards (0x09A7: di != 0 -> 0x09AE exit_game). */
void race_victory_df58(void);

/* ---- internals shared by race.c and race_gfx.c (they define RACE_INTERNAL; glad.h has its own REC / R_BODY) ---- */
#ifdef RACE_INTERNAL
/* racer record i (0 Red, 1 Green, 2 Blue, 3 the player): DS:C2BE + 18i */
#define REC(i, o)   ((uint16_t)(0xC2BE + 0x12 * (unsigned)(i) + (o)))
#define R_POS    0x00   /* w  position in the lap, 75 units per path point */
#define R_RUN    0x04   /* w  running (FFFF) / out (0) */
#define R_LAPS   0x06   /* sb laps (starts at -1) */
#define R_PROV   0x07   /* b  home province */
#define R_TYPE   0x08   /* b  chariot type 0 light, 1 medium, 2 heavy */
#define R_TOP    0x09   /* b  top speed */
#define R_SPEED  0x0A   /* b  speed */
#define R_STEP   0x0B   /* b  path units per tick */
#define R_SKID   0x0C   /* sb skid 0..15 */
#define R_LAST   0x0D   /* b  action taken at the last animation boundary */
#define R_CMD    0x0E   /* b  pending command */
#define R_BODY   0x0F   /* sb chariot strength */
#define R_HORSE  0x10   /* sb horses' strength */
#define R_CRASH  0x11   /* b  crash counter */
#define PREV(i)  ((uint16_t)(0xC30A + 4 * (unsigned)(i)))   /* w  path point, previous tick */
#define PATH(i)  ((uint16_t)(0xC30C + 4 * (unsigned)(i)))   /* w  path point */
#define FIN(i)   ((uint16_t)(0xC31A + 2 * (unsigned)(i)))   /* w  finish tick (FFFF = none) */
#define REST(i)  ((uint16_t)(0xC324 + 2 * (unsigned)(i)))   /* w  tick of the last whip / horse bump */
#define LANE(i)  ((uint16_t)(0xC334 + 4 * (unsigned)(i)))   /* w  lane x 0..280 */
#define SCRY(i)  ((uint16_t)(0xC336 + 4 * (unsigned)(i)))   /* w  screen y */
#define ROOT(i)  W(0x83CC + 2 * (unsigned)(i))              /* the racer's sprite tree: the horses' node */
/* sprite-tree node (16 bytes, DS:808C..838B) */
#define N_NEXT 0x0
#define N_CHILD 0x2
#define N_DESC 0x4      /* w  frame descriptor of frame 0 (a sheet value below + 14 * frame) */
#define N_OTAB 0x6      /* w  offset table: s8 dx, dy per frame */
#define N_DX   0x8
#define N_DY   0x9
#define N_ACT  0xA      /* w  active */
#define N_CNT  0xC      /* b  frames */
#define N_FR   0xD      /* b  current frame */
#define N_TK   0xE      /* b  tick */
#define N_PER  0xF      /* b  ticks per frame (see race.c, the BD0A note) */
/* Sheet values in the DS words C2BC (race), C306 (laps), C352 (med), C354 (race2): the original keeps far pointers to
   14-byte descriptors and adds 14 * frame. The port stores (naval_gfx.c handle << 12), so a descriptor value is
   (handle << 12) + 14 * frame and the original's additions (+ 0x46, + 0x118, 14 * frame ...) work unchanged. 0 = none. */
#define RC_SHEET(v)   ((uint16_t)(v) >> 12)
#define RC_FRAME(v)   (((uint16_t)(v) & 0x0FFF) / 14)
#define RC_GALLERY1   0xC356u           /* the descriptor of gallery1.cbm (8E62(&C356, 0, 0)) */

/* race_gfx.c */
int  rc_archive_0F7B(void);             /* archive 0 "mrace": 1 = open */
uint16_t rc_sheet_163B(const char *name);   /* a sheet value (handle << 12) or 0 */
void rc_sheet_19A0(uint16_t v);
int  rc_picture_13A8(void);             /* raceest.cbm into picture slot 3: 1 = loaded */
int  rc_gallery1_1273(void);            /* gallery1.cbm into the descriptor C356: 1 = loaded */
void rc_gallery1_free_86A0(void);
int  rc_circus_load(void);              /* circus.cbm (the local descriptor of C2DE): 1 = loaded */
void rc_circus_draw(void);              /* 8E62(&circus, 0, 0) */
void rc_circus_free(void);              /* 86A0 */
void rc_sprite_8E62(uint16_t desc, int x, int y);
void rc_blit_8790(void);                /* raceest at the point DS:840E, opaque */
void rc_pixel_82DE(int x, int y);
void rc_line_979C(int x0, int y0, int x1, int y1);
void rc_text_899C(const char *s, int x, int y);
void rc_plot_DE64(uint16_t pat, int16_t y, int16_t x);
void rc_card_CDCE(int i);
void rc_laps_D392(void);
void rc_gauges_D3C2(int all);
void rc_speed_D400(void);
void rc_horses_D49A(void);
void rc_body_D4EC(void);
void rc_chariot_C23D(int type, int x, int f);
void rc_brazier_C299(void);
void rc_path_point(uint16_t p, int *x, int *y);    /* the path buffer W(C308) (calloc'd by CBE6): point p */
void rc_path_set(uint16_t p, int16_t x, int16_t y);
void rc_path_alloc(void);
void rc_path_free(void);

#endif /* RACE_INTERNAL */
#endif
