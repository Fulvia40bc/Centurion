#ifndef STATE_H
#define STATE_H
#include <stdint.h>

/* Image of the original's data segment; see state.c. */
extern uint8_t g_ds[0x10000];

/* Accessors with the original's widths and signedness. */
#define B(a)   (g_ds[(uint16_t)(a)])                                  /* u8  */
#define SB(a)  ((int8_t)g_ds[(uint16_t)(a)])                          /* s8 (cbw/cwde) */
#define W(a)   ((uint16_t)(g_ds[(uint16_t)(a)] | (g_ds[(uint16_t)((a) + 1)] << 8)))
#define SW(a)  ((int16_t)W(a))
static inline void setW(uint16_t a, uint16_t v) { g_ds[a] = (uint8_t)v; g_ds[(uint16_t)(a + 1)] = (uint8_t)(v >> 8); }

int state_init(const char *ovl0_path);   /* DS image from OVL0.OVL (file 0xB400 = DS:0000) */
int state_load(const char *path);        /* .CSV -> DS via the save block table at DS:5BDE */
int state_save(const char *path);        /* DS -> .CSV */
int state_load_mem(const char *path);    /* DS from a 640K memory snapshot (DS = 1686:0000) */

void end_turn(void);                     /* EXE 0x09BA, see endturn.c */
int  add_cap(int v, int step, int max);  /* EXE 0x04FA */
void score_add(int n);                   /* EXE 0x7330 */

/* enemy.c */
void     rng_seed(uint32_t ticks);       /* EXE 0x075A */
uint16_t rng_roll(uint16_t n);           /* EXE 0x076B */
void     enemy_turn(void);               /* EXE 0x4C24 */
void     random_events(uint16_t tribute);/* EXE 0x1E98, events.c */
extern int g_original_bugs;             /* --original: keep the 1990 bugs (byte-exact checks) */
extern int g_enemy_unported;            /* set when a battle or other unported path was reached */

/* battle.c: the land battle (OVL0 section 2) */
#include <stddef.h>
void     legion_lost(int j);             /* EXE 0x4816, enemy.c */
uint16_t battle(int A, int L, int Z, uint8_t pf, uint8_t pt, const uint8_t *tactics, size_t tactics_len);
uint16_t battle_from_formation(int A, int L, int Z, uint8_t pf, uint8_t pt);
extern int (*battle_orders_hook)(int tick);
extern int g_battle_quirks;
void     battle_order_legion_melee(void);     /* orders mode: "Release The Legion!" */
void     battle_order_legion_retreat(void);   /* "Run Away! Run Away!" */
int      battle_in_voice(int i);
int      battle_cmd_begin(int i, uint8_t from_x, uint8_t from_y);   /* from_x 0xFF = keyboard (cursor on the unit) */
void     battle_cmd_target(uint8_t x, uint8_t y);
void     battle_cmd_hold(void);
void     battle_cmd_melee(void);
void     battle_cmd_retreat(void);
void     battle_cmd_end(void);
void     assign_commander(int L);
extern int  (*battle_menu_hook)(int which);
extern void (*battle_setup_hook)(int Z);
extern void (*battle_message_hook)(const char *s);
extern void (*battle_report_hook)(int won, const char *title, const char *text, unsigned rcas, unsigned ecas, unsigned rsurv, unsigned esurv);
void     battle_sprites_build(void);     /* BB4D */
void     roman_numeral(unsigned n, char *out);
void     battle_report_text(int L, int won, char *out, size_t n);   /* EXE 0x6738 */
/* battle_ui.c */
uint16_t battle_play(int A, int L, int Z);   /* on screen (platform_init first) */
uint16_t battle_ui_shots(int A, int L, int Z, int pf, int pt, int shot_tick, const char *prefix);
void     battle_report_shot(int L, int won, unsigned rc, unsigned ec, unsigned rs, unsigned es, const uint8_t *bg, const char *out);
/* battle_gfx.c */
int      battle_gfx_load(int Z);         /* BB97: field + sprite sheets (BATL.DAT loaded, LUTs loaded) */
void     battle_sort_sprites(void);      /* BA4F */
void     battle_draw(uint8_t *fb);       /* BB7E + BA72 */
void     battle_grid_to_screen(int x, int y, int *sx, int *sy);        /* EXE 0x4733 */
extern int (*battle_choices)(uint8_t *formation, uint8_t *tactic);
int      battle_auto(int A, int L, int Z, uint16_t *won);   /* 0 = no choices available (not fought) */

#endif
