#ifndef GLAD_H
#define GLAD_H
/* The gladiator show, OVL0 section 3 (file 0x3EF0-0x51A0, loaded at 0823:B0E0); disasm/OVL_GLAD.asm, docs/GLAD_INDEX.md,
 * GLAD_LOGIC_NOTES.md, GLAD_SCREEN_NOTES.md, GLAD_DATA_NOTES.md. Logic, timing and dice in glad.c, the drawing in glad_gfx.c
 * (on top of naval_gfx.c's EXE helpers: targets, fills, text, the sprite-sheet loader and blitter). */
#include <stdint.h>

/* section 3 B0E0 via thunk B0B6 (Rome Games 0x34F5): the whole show; no arguments, no result */
void glad_show_b0e0(void);

/* ---- internals shared by glad.c and glad_gfx.c ---- */
/* the two fighter blocks (0x18 bytes each): P = the player's man (right), O = the opponent (left) */
#define GL_P        0xAA52u
#define GL_O        0xAA6Au
#define F_TAB(f)    ((uint16_t)((f) + 0))   /* w  move table: 4-byte states {dx (signed), next, frame, flags} */
#define F_X(f)      ((uint16_t)((f) + 2))   /* w  x */
#define F_Y(f)      ((uint16_t)((f) + 4))   /* w  y */
#define F_ST(f)     ((uint16_t)((f) + 6))   /* b  state (FF = finished falling) */
#define F_ST2(f)    ((uint16_t)((f) + 7))   /* b  state copy (P only, never read) */
#define F_TRN(f)    ((uint16_t)((f) + 8))   /* b  training 0..3 */
#define F_DIE(f)    ((uint16_t)((f) + 9))   /* b  dying state */
#define F_HP(f)     ((uint16_t)((f) + 10))  /* sb health */
/* gladiator record r (0..9 men, saved; 10..12 the beasts, not saved): DS:7E3E + 10r */
#define REC(r, o)   ((uint16_t)(0x7E3E + 10 * (unsigned)(r) + (o)))
#define R_NAME 0    /* w  name pointer */
#define R_GEN  2    /* b  generation (1 = no numeral) */
#define R_WON  3    /* b  bouts won */
#define R_LOST 4    /* b  bouts lost (never shown) */
#define R_STAT 5    /* b  0 free, 1 picked, 3 in the line-up */
#define R_STY  6    /* b  style 0..3, 4 = beast */
#define R_BODY 7    /* b  0 man, 1 wimp, 2..4 cats */
#define R_TRN  8    /* b  training bought 0..3 */
#define SH_ARENA   0xC2B4u                  /* sheet handles (naval_gfx.c's small handles, as the naval battle) */
#define SH_PLAYER  0xC2B6u
#define SH_OPP     0xC2B8u
#define SH_SHOW    0xC2BAu

/* glad.c */
uint8_t gl_mt(uint16_t f, int k);           /* byte k (0 dx, 1 next, 2 frame) of fighter f's current state */
uint8_t gl_flags(uint16_t f);               /* its flags (with the O man table fix unless --original) */
/* glad_gfx.c */
int  gl_picture_13A8(void);                 /* arenabtm.cbm into "picture slot 4" (1 = loaded) */
int  gl_title_1273(void);                   /* coloseum.cbm: 1 = there (gl_title_draw draws it) */
void gl_title_draw(void);                   /* B11B..B154 */
void gl_arena_draw(void);                   /* B29B..B2DC */
void gl_grid_B67C(void);
void gl_card_B6EC(int pos, uint8_t rec, int facing);
void gl_crowd_C149(int v);
void gl_fighters_C1A4(void);
void gl_health_C25F(int side);
void gl_box_3233(int x0, int y0, int x1, int y1, int c);
void gl_thumbs_box(void);                   /* BFD8: arena frame 3 at (79, 20) */
void gl_pictures_free(void);

#endif
