/* Module STACK of docs/MAP_PLAN.md: the World State Stack of AC_MAIN.EXE and the in-game SAVE / RESTORE / NEW GAME /
   REPLAY buttons:
     0x121A0 free the game sprites, 0x7A8D0 allocate the World State Stack, 0x7A940 push, 0x7ABD0 free, 0x7AC60 pop,
     and the dispatcher 0x3FB94's cases 0x4F (SAVE GAME, -> 0x3FDAB), 0x50 (RESTORE GAME, -> 0x3FEA0), 0x51 (START NEW
     GAME, -> 0x3FDCF) and 0x101 (REPLAY, -> 0x4041D) as dcase_<id>. Each jump-table entry (0x3F780 + 4 * (id - 3):
     0x3F8B0, 0x3F8B4, 0x3F8B8, 0x3FB78) was read in obj1.bin and equals its off32 fixup's target.
   Literal translations; the address of each instruction a line comes from is in its comment. D = read in the code,
   G = inference. Prototypes: stack.h.

   The World State Stack (D):
     int32 [0xCAC54]  depth, -1 = empty (initialised data: 0xFFFFFFFF in obj2); slot d is the d-th push (0..9)
     int32 [0x112A8C] the deepest it has been (BSS; G: printed by the developer statistics "Max WorldStack: %d", 0xB0F9E)
     uint32 [0xF5240 + 4i], i 0..9  0x2B20-byte blocks: area records 1..160 (0xE31FD = 0xE31B8 + 69, 160 * 69 bytes)
     uint32 [0xF5268 + 4i], i 0..9  0x88D-byte blocks: nation records 1..11 (0xF1593 = 0xF14CC + 199, 11 * 199 bytes)
     uint32 [0xF5290 + 4i], i 0..9  0x1E0-byte blocks: the 160 three-byte territory-state records at 0xE2C18
   The 30 fields hold port heap handles (load_heap_alloc / load_heap_ptr / load_heap_free), never C pointers (by
   design against a DOSBox-X dump, MAP_PLAN risk 6). The push needs no allocation itself: 0x7A8D0 must have run (G:
   the AI's trial moves allocate it; the only caller ported here is REPLAY). A block that does not resolve (the
   original would copy through a null or freed pointer) stops the port with a message (port guard, G).

   REPLAY's data (D):
     int32 [0xEDF58]  1 = a post-movement snapshot exists (0x54AAE, at the end of movement; saved and loaded with the
                      game, 0x5EF4A / 0x5F755; cleared for a new game by 0x12DA4)
     int32 [0xEDF5C]  its round (int16 [0xF40FC] at 0x54AA9)
     0xE5D20  161 area records (0x2B65 bytes, records 0..160, copied from 0xE31B8 at 0x54A42)
     0xE2DF8  the 160 territory-state records (0x1E0, from 0xE2C18 at 0x54A66)
     0xF1FB0  14 nation records (0xAE2 bytes, records 0..13, from 0xF14CC at 0x54A85)
     int16 [0x107692]  1 while a replay is showing (REPLAY pressed again then returns -1, which ends the replay's wait)
     the message area int16 0xBB3FA x, 0xBB3FC y, 0xBB3FE w, 0xBB400 h (start.h); 0x109E7C an 8-byte sprintf buffer
       (0x109E84 is the play-by-email name) used as REPLAY's label: the round number ("%d", 0xB401B / 0xB4046)
     dialog 0xF596C (the game panel's buttons): during a replay only REPLAY 0x101, TOGGLE UNITS 0x24, the mini-map
       0x23 and the four straight scroll arrows 0x1C, 0x1E, 0x1F, 0x21 are enabled

   NEW GAME / RESTORE free these heap fields (D), each only if non-0, and zero it: 0xF0984[1..11] (the save's record
   lists, saveload.c; flag 0x01 of [0xE1624]), [0xEED74], [0xEED7C], [0xEED78], [0xEED80] (G: a saved copy of the
   areas / nations / territory state, 0x7AB24, and one more block; allocated by code not ported yet). Port handles:
   load_heap_free, which ignores 0 and anything that is not a live handle.

   The longjmps (D): case 0x51 ends in longjmp(0xF58D4, 1) (0x3FE96 mov eax, 0xF58D4; 0x3FE9B jmp 0x87879, edx = 1 from
   0x3FE8C, kept across f3C7A0), main's setjmp at 0x11036 (the title menu); case 0x50 in longjmp(0xF58A0, 1) (0x3FF7D,
   0x3FF82, edx = 1 from 0x3FF73), main's setjmp at 0x115CE (the game screen after the maps and sprites). The port
   calls port_longjmp (stack.h; main.c implements it). Main tests neither setjmp's return value: after the jump it
   runs on exactly as after the first return (0x1103B, 0x115D3).

   Runtime calls (Watcom C runtime -> standard C): 0x822B5 malloc -> load_heap_alloc; 0x877E6 free -> load_heap_free
   (for a sprite record: f3AF54); 0x7DB3D sprintf -> snprintf bounded to the buffer (0xF5500: 0x1F4 as elsewhere in the
   port; 0x109E7C: 8 bytes) with the original's format from the data object; the inline rep movsd / movsb copies ->
   memcpy; the inline byte-pair strcpy (0x7ACFD) -> a bounded copy. __CHK dropped.
   fg_makegif / fg_showgif "junk" (0x4049A / 0x406BA): kept in memory as in info.c / menu1.c / menu2.c (MAP_PLAN risk
   7; this is a fourth copy of that helper, to merge later): the rectangle of the active surface is read with
   fg_getpixel and put back with fg_point at the cursor, opaque, the colour restored. No junk file is written.

   1995 bugs (fixed unless g_original, each marked BUG(1995) below):
     1. 0x121A0 frees 28 kinds of game sprites but not three that 0x11A8C cuts: the 13 tokens2.gif sprites 0xF4D5C +
        8i (i 1..13, 11x10 at y 427) and the two tokens.gif sprites 0xF519C and 0xF51A4 (27x13). After START NEW GAME
        (and on every later game start, which runs 0x11A8C again) their blocks are lost (D: the free list against
        gfxload.c's cut list; G: about 2 KB each time, harmless in practice). Fixed: freed too, after the original's
        list. The fix also leaves those 15 records' +4 width 0 until 0x11A8C cuts them again, as on a fresh start.
     2. REPLAY copies the snapshot with the record-0 bases (0x2B20 bytes 0xE5D20 -> 0xE31B8 = area records 0..159;
        0x88D bytes 0xF1FB0 -> 0xF14CC = nation records 0..10), while the stack it pushes before and pops after
        covers records 1..160 and 1..11 (0xE31FD, 0xF1593), the snapshot holds 0..160 and 0..13, and record 0 is
        "none" for areas (game.h). So area record 160 and nation record 11 are not shown in their post-movement
        state, and area record 0 and nation record 0 (a scratch hand of the trade code, f6DD98(0) at 0x69B59, G) keep
        the snapshot's bytes after the replay instead of being put back (D: the bases and sizes; G: whether those
        records ever differ, so whether anything shows). Fixed: records 1..160 and 1..11 are copied (0xE5D65 ->
        0xE31FD, 0xF2077 -> 0xF1593), exactly what the stack saves and restores: the replay then leaves the game
        state as it found it.
   Text slip (fixed unless g_original, docs/TEXT_SWEEP.md): "popped once to often" -> "too often" (0xB9679).
   Oddities kept (D):
     - RESTORE (0x50) frees the record lists 0xF0984[1..11] that the restore 0x5FF28 (0x5F05C) has just read from
       the save (only when flag 0x01 of [0xE1624] is set; G: play-by-email move logs, which the port drops by Erik's
       ruling). It copies NEW GAME's clean-up, which runs before nothing is loaded. Kept: PBEM only.
     - RESTORE does not free the sprites (0x121A0) or the map blocks (0x3F558): main's setjmp 0x115CE lies after the
       loading of both, so they are kept, but main cuts the minimap 0xF520C again at 0x11628 (and loads N.RL again
       at 0x116E1, G) without freeing the old block: main's code, noted for main.c.
     - The push's "> %d" prints 10, the slot count; the push refuses at depth 9 (all ten slots in use). Consistent. */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fg.h"
#include "game.h"
#include "map.h"
#include "mapdata.h"
#include "panel.h"
#include "save.h"
#include "stack.h"
#include "state.h"
#include "ui.h"

#define DEPTH      0xCAC54u    /* int32 the stack's depth, -1 = empty */
#define MAXDEPTH   0x112A8Cu   /* int32 the deepest it has been */
#define SLOT_AREA  0xF5240u    /* uint32[10] handles of 0x2B20-byte blocks */
#define SLOT_NAT   0xF5268u    /* uint32[10] handles of 0x88D-byte blocks */
#define SLOT_TST   0xF5290u    /* uint32[10] handles of 0x1E0-byte blocks */
#define AREA1      0xE31FDu    /* area record 1 (0xE31B8 + 69) */
#define NAT1       0xF1593u    /* nation record 1 (0xF14CC + 199) */
#define TSTATE     0xE2C18u
#define SZ_AREA    0x2B20u
#define SZ_NAT     0x88Du
#define SZ_TST     0x1E0u
#define MSG        0xF5500u    /* the quit message buffer */
#define CURDLG     0xF8E78u    /* uint32 the current dialog */
#define PANEL_DLG  0xF596Cu    /* the game panel's dialog */
#define LABEL      0x109E7Cu   /* 8-byte sprintf buffer, REPLAY's label */
#define REPLAYING  0x107692u   /* int16 1 while a replay shows */
#define BACKBUF    0xF52C8u    /* int32 virtual-buffer handle of the back buffer */

/* ---- the "junk" GIF, kept in memory (file comment; as info.c's info_makegif / info_showgif) ------------------- */

typedef struct {
    int valid;
    int32_t w, h;
    uint8_t px[640 * 480];
} stack_gif;

static stack_gif s_junk;     /* "junk" (written as 0xB4016, read as 0xB4049) */

/* fg_makegif(minx, maxx, miny, maxy, name) (0x8CE50): the rectangle of the active surface */
static void stack_makegif(int minx, int maxx, int miny, int maxy, stack_gif *g)
{
    int32_t w = maxx - minx + 1, h = maxy - miny + 1, r, c, v;

    if (w <= 0 || h <= 0 || (int64_t)w * h > (int64_t)sizeof g->px) {
        g->valid = 0;                         /* port guard (G): the game's rectangles are all inside the screen */
        return;
    }
    for (r = 0; r < h; r++)
        for (c = 0; c < w; c++) {
            v = fg_getpixel(minx + c, miny + r);
            g->px[r * w + c] = (uint8_t)(v < 0 ? 0 : v);
        }
    g->w = w;
    g->h = h;
    g->valid = 1;
}

/* fg_showgif(name, 3) (0x88F98): at the cursor, palette kept, opaque; colour and cursor unchanged. No file: nothing
   drawn (the original returns 1). */
static void stack_showgif(const stack_gif *g)
{
    int32_t x0 = fg_getxpos(), y0 = fg_getypos(), r, c;
    int color = fg_getcolor();

    if (!g->valid)
        return;
    for (r = 0; r < g->h; r++)
        for (c = 0; c < g->w; c++) {
            fg_setcolor(g->px[r * g->w + c]);
            fg_point(x0 + c, y0 + r);
        }
    fg_setcolor(color);
}

/* The block of slot field (a handle), n bytes. Port guard (G): the original copies through whatever the field
   holds (null if 0x7A8D0 never ran, or a freed block); the port quits with a message instead. */
static uint8_t *slot_block(uint32_t field, uint32_t n)
{
    uint8_t *p = load_heap_ptr(D32(field), n);

    if (p == NULL) {
        snprintf((char *)DP(MSG), 0x1F4, "World State Stack slot 0x%X has no block", (unsigned)field);
        f44CAC();
    }
    return p;
}

/* ---- 0x121A0: free the game sprites --------------------------------------------------------------------------- */

void f121A0(void)
{
    int32_t i;                                                      /* [ebp-0x14] */

    f3AF54(0xF5214u);                                               /* 0x121B9 c_invld */
    f3AF54(0xF521Cu);                                               /* 0x121C3 */
    f3AF54(0xF5224u);                                               /* 0x121D2 (edx = 1 for the loop, 0x121CD) */
    for (i = 1; i <= 0xD; i++) {                                    /* 0x121D7, 0x121E2 cmp 0xD; jg */
        f3AF54(0xF489Cu + 8u * (uint32_t)i);                        /* 0x121F3 */
        f3AF54(0xF497Cu + 8u * (uint32_t)i);                        /* 0x12203 */
        f3AF54(0xF4E3Cu + 8u * (uint32_t)i);                        /* 0x12213 */
        f3AF54(0xF4DCCu + 8u * (uint32_t)i);                        /* 0x12223 */
        if (i <= 9)                                                 /* 0x1222B cmp esi, 9; jg 0x121DC */
            f3AF54(0xF490Cu + 8u * (uint32_t)i);                    /* 0x1223B */
    }
    for (i = 1; i < 0xE; i++) {                                     /* 0x12242, 0x12251 cmp 0xE; jge */
        f3AF54(0xF4ACCu + 8u * (uint32_t)i);                        /* 0x12262 */
        f3AF54(0xF4BACu + 8u * (uint32_t)i);                        /* 0x12272 */
        f3AF54(0xF4B3Cu + 8u * (uint32_t)i);                        /* 0x12282 */
        f3AF54(0xF49ECu + 8u * (uint32_t)i);                        /* 0x12292 */
        if (i <= 9)                                                 /* 0x1229A cmp ebx, 9; jg 0x1224B */
            f3AF54(0xF4A5Cu + 8u * (uint32_t)i);                    /* 0x122AA */
    }
    f3AF54(0xF4D2Cu);                                               /* 0x122B6 */
    f3AF54(0xF4D34u);                                               /* 0x122C0 */
    f3AF54(0xF4D3Cu);                                               /* 0x122CA */
    f3AF54(0xF4D44u);                                               /* 0x122D4 */
    f3AF54(0xF4D4Cu);                                               /* 0x122DE */
    f3AF54(0xF4D54u);                                               /* 0x122E8 */
    f3AF54(GAME_SPR_MINIMAP);                                       /* 0x122F2 0xF520C */
    f3AF54(0xF518Cu);                                               /* 0x122FC */
    f3AF54(0xF5194u);                                               /* 0x12306 */
    f3AF54(0xF517Cu);                                               /* 0x12310 */
    f3AF54(0xF5184u);                                               /* 0x1231C (ecx = 0 for the loop, 0x1231A) */
    for (i = 0; i < 0x1E; i++) {                                    /* 0x12321, 0x1232C cmp 0x1E; jge */
        f3AF54(0xF4EACu + 8u * (uint32_t)i);                        /* 0x1233D */
        f3AF54(0xF4F9Cu + 8u * (uint32_t)i);                        /* 0x1234D */
        f3AF54(0xF508Cu + 8u * (uint32_t)i);                        /* 0x1235D */
    }
    for (i = 1; i <= 0x21; i++)                                     /* 0x12364, 0x12373 cmp 0x21; jg */
        f3AF54(GAME_SPR_CARDTINY + 8u * (uint32_t)i);               /* 0x12384 0xF4C1C + 8i */

    if (!g_original) {
        /* BUG(1995) 1: the original never frees these three kinds, which 0x11A8C cuts again on the next game start
           (0x11C4D, 0x11ECE, 0x11EE9): their blocks are lost. Fixed: freed with the others. */
        for (i = 1; i <= 0xD; i++)
            f3AF54(0xF4D5Cu + 8u * (uint32_t)i);
        f3AF54(0xF519Cu);
        f3AF54(0xF51A4u);
    }
}                                                                   /* 0x1238B */

/* ---- 0x7A8D0: allocate the World State Stack ------------------------------------------------------------------ */

void f7A8D0(void)
{
    int32_t i;                                                      /* [ebp-0x8] */

    if (D32(SLOT_AREA) != 0)                                        /* 0x7A8E1 mov edx, [0xF5240]; test; jne 0x7A93A */
        return;
    for (i = 0; i < 0xA; i++) {                                     /* 0x7A8EB (edx = 0), 0x7A8F6 cmp 0xA; jge */
        setD32(SLOT_AREA + 4u * (uint32_t)i, load_heap_alloc(SZ_AREA));   /* 0x7A8FC malloc(0x2B20), 0x7A909 */
        setD32(SLOT_NAT + 4u * (uint32_t)i, load_heap_alloc(SZ_NAT));     /* 0x7A910 malloc(0x88D), 0x7A91D */
        setD32(SLOT_TST + 4u * (uint32_t)i, load_heap_alloc(SZ_TST));     /* 0x7A924 malloc(0x1E0), 0x7A931 */
    }
}                                                                   /* 0x7A93A */

/* ---- 0x7A940: push -------------------------------------------------------------------------------------------- */

void f7A940(void)
{
    int32_t d = S32(DEPTH);                                         /* 0x7A952 edx = [0xCAC54] */
    int32_t nd;

    if (d >= 9) {                                                   /* 0x7A958 cmp edx, 9; jge 0x7A9F9 (signed) */
        snprintf((char *)DP(MSG), 0x1F4, DSTR(0xB961Du), 0xA);    /* 0x7AA05 sprintf(0xF5500, "World State Stack
                                                                       has blown its max size. > %d", 10) */
        f44CAC();                                                   /* 0x7AA0D quit; the original returns after it */
    }
    nd = d + 1;                                                     /* 0x7A961 lea ebx, [edx+1] */
    setD32(DEPTH, (uint32_t)nd);                                    /* 0x7A96A */
    if (nd > S32(MAXDEPTH))                                         /* 0x7A970 cmp ebx, ecx; jle (signed) */
        setD32(MAXDEPTH, (uint32_t)nd);                             /* 0x7A974 */

    d = S32(DEPTH);                                                 /* 0x7A97A */
    memcpy(slot_block(SLOT_AREA + 4u * (uint32_t)d, SZ_AREA), DP(AREA1), SZ_AREA);   /* 0x7A980..0x7A99E */
    d = S32(DEPTH);                                                 /* 0x7A9A1 */
    memcpy(slot_block(SLOT_TST + 4u * (uint32_t)d, SZ_TST), DP(TSTATE), SZ_TST);     /* 0x7A9A7..0x7A9C5 */
    d = S32(DEPTH);                                                 /* 0x7A9C8 */
    memcpy(slot_block(SLOT_NAT + 4u * (uint32_t)d, SZ_NAT), DP(NAT1), SZ_NAT);       /* 0x7A9CE..0x7A9EC */
}                                                                   /* 0x7A9EF */

/* ---- 0x7ABD0: free the World State Stack ---------------------------------------------------------------------- */

void f7ABD0(void)
{
    int32_t i;                                                      /* [ebp-0xC] */

    if (D32(SLOT_AREA) == 0)                                        /* 0x7ABE2 cmp [0xF5240], 0; je 0x7AC57 */
        return;
    for (i = 0; i < 0xA; i++) {                                     /* 0x7ABEB, 0x7ABF8 cmp 0xA; jge */
        load_heap_free(D32(SLOT_AREA + 4u * (uint32_t)i));          /* 0x7AC08 free */
        setD32(SLOT_AREA + 4u * (uint32_t)i, 0);                    /* 0x7AC12 (edi = 0) */
        load_heap_free(D32(SLOT_NAT + 4u * (uint32_t)i));           /* 0x7AC23 */
        setD32(SLOT_NAT + 4u * (uint32_t)i, 0);                     /* 0x7AC2B */
        load_heap_free(D32(SLOT_TST + 4u * (uint32_t)i));           /* 0x7AC3C */
        setD32(SLOT_TST + 4u * (uint32_t)i, 0);                     /* 0x7AC44 */
    }
    setD32(DEPTH, 0xFFFFFFFFu);                                     /* 0x7AC4D depth = -1 */
}                                                                   /* 0x7AC57 */

/* ---- 0x7AC60: pop --------------------------------------------------------------------------------------------- */

void f7AC60(void)
{
    int32_t d = S32(DEPTH);                                         /* 0x7AC71 edx = [0xCAC54] */
    const char *text;

    if (d < 0) {                                                    /* 0x7AC77 test edx, edx; jl 0x7ACF2 */
        /* 0x7ACF2..0x7AD15 the inline strcpy of "World State Stack has popped once to often." (0xB9679) */
        text = g_original ? DSTR(0xB9679u)
                          : "World State Stack has popped once too often.";   /* text slip fixed (TEXT_SWEEP.md) */
        snprintf((char *)DP(MSG), 0x1F4, "%s", text);
        f44CAC();                                                   /* 0x7AD16 quit; the original returns after it */
    }
    memcpy(DP(AREA1), slot_block(SLOT_AREA + 4u * (uint32_t)d, SZ_AREA), SZ_AREA);   /* 0x7AC7B..0x7AC99 */
    d = S32(DEPTH);                                                 /* 0x7AC9C */
    memcpy(DP(TSTATE), slot_block(SLOT_TST + 4u * (uint32_t)d, SZ_TST), SZ_TST);     /* 0x7ACA2..0x7ACC0 */
    d = S32(DEPTH);                                                 /* 0x7ACC3 */
    memcpy(DP(NAT1), slot_block(SLOT_NAT + 4u * (uint32_t)d, SZ_NAT), SZ_NAT);       /* 0x7ACC9..0x7ACE7 */
    setD32(DEPTH, (uint32_t)(S32(DEPTH) - 1));                      /* 0x7ACEA dec [0xCAC54] */
}                                                                   /* 0x7AD1B */

/* ---- the frees NEW GAME and RESTORE share (0x3FDE8..0x3FE81 = 0x3FED1..0x3FF68) ------------------------------- */

static void free_game_blocks(void)
{
    int32_t i;                                                      /* [ebp-0x40] */

    for (i = 1; i <= 0xB; i++) {                                    /* 0x3FDE8 / 0x3FED1 (i = 1), cmp 0xB; jg */
        if (D32(0xF0984u + 4u * (uint32_t)i) != 0) {                /* 0x3FDFC / 0x3FEE5 test; je */
            load_heap_free(D32(0xF0984u + 4u * (uint32_t)i));       /* 0x3FE11 / 0x3FEFA free */
            setD32(0xF0984u + 4u * (uint32_t)i, 0);                 /* 0x3FE1B / 0x3FF04 */
        }
    }
    if (D32(0xEED74u) != 0) {                                       /* 0x3FE24 / 0x3FF0D */
        load_heap_free(D32(0xEED74u));                              /* 0x3FE32 / 0x3FF1B */
        setD32(0xEED74u, 0);                                        /* 0x3FE37 / 0x3FF20 */
    }
    if (D32(0xEED7Cu) != 0) {                                       /* 0x3FE3D / 0x3FF26 */
        load_heap_free(D32(0xEED7Cu));                              /* 0x3FE49 / 0x3FF31 */
        setD32(0xEED7Cu, 0);                                        /* 0x3FE50 / 0x3FF36 */
    }
    if (D32(0xEED78u) != 0) {                                       /* 0x3FE55 / 0x3FF3C */
        load_heap_free(D32(0xEED78u));                              /* 0x3FE63 / 0x3FF4A */
        setD32(0xEED78u, 0);                                        /* 0x3FE68 / 0x3FF4F */
    }
    if (D32(0xEED80u) != 0) {                                       /* 0x3FE6E / 0x3FF55 */
        load_heap_free(D32(0xEED80u));                              /* 0x3FE7C / 0x3FF63 */
        setD32(0xEED80u, 0);                                        /* 0x3FE81 / 0x3FF68 */
    }
}

/* ---- the dispatcher cases -------------------------------------------------------------------------------------- */

int dcase_4F(int code)                                              /* -> 0x3FDAB SAVE GAME */
{
    f3CC68();                                                       /* 0x3FDAB the background buffer on the screen */
    f3CCBC();                                                       /* 0x3FDB0 the screen into the back buffer */
    f5E7D0();                                                       /* 0x3FDBA (ecx = -1, 0x3FDB5) */
    code = -1;                                                      /* 0x3FDBF mov [ebp-0x3C], ecx */
    return code;                                                    /* 0x3FDC2 */
}

int dcase_51(int code)                                              /* -> 0x3FDCF START NEW GAME */
{
    (void)code;
    f3CC68();                                                       /* 0x3FDCF */
    f121A0();                                                       /* 0x3FDD4 the game sprites */
    f7ABD0();                                                       /* 0x3FDD9 the World State Stack */
    f3F558();                                                       /* 0x3FDE3 the map blocks (esi = 1, 0x3FDDE) */
    free_game_blocks();                                             /* 0x3FDE8..0x3FE81 */
    f3C7A0(D32(CURDLG));                                            /* 0x3FE91 f3C7A0([0xF8E78]) (edx = 1, 0x3FE8C) */
    port_longjmp(0xF58D4u);                                         /* 0x3FE96 mov eax, 0xF58D4; 0x3FE9B jmp 0x87879:
                                                                       longjmp(0xF58D4, 1), main 0x11036 */
}

int dcase_50(int code)                                              /* -> 0x3FEA0 RESTORE GAME */
{
    f3CC68();                                                       /* 0x3FEA0 */
    f3CCBC();                                                       /* 0x3FEA5 */
    if (f5FF28() == -15) {                                          /* 0x3FEAA; 0x3FEAF cmp eax, -0xF; je 0x3FD35 */
        code = -1;                                                  /* 0x3FD35 mov [ebp-0x3C], 0xFFFFFFFF */
        return code;
    }
    setD8(0xE1624u, (uint8_t)(D8(0xE1624u) & 0xFB));                /* 0x3FEB8..0x3FEC6 and bl, -5: not a new game */
    f7ABD0();                                                       /* 0x3FECC (edi = 1, 0x3FEC1) */
    free_game_blocks();                                             /* 0x3FED1..0x3FF68 */
    f3C7A0(D32(CURDLG));                                            /* 0x3FF78 f3C7A0([0xF8E78]) (edx = 1, 0x3FF73) */
    port_longjmp(0xF58A0u);                                         /* 0x3FF7D mov eax, 0xF58A0; 0x3FF82 jmp 0x87879:
                                                                       longjmp(0xF58A0, 1), main 0x115CE */
}

int dcase_101(int code)                                             /* -> 0x4041D REPLAY */
{
    int32_t h, y;                                                   /* [ebp-0x2C], [ebp-0x28] */
    int32_t allocated;                                              /* [ebp-0x38] */
    int32_t x, w;

    if (S32(0xEDF58u) == 0)                                         /* 0x4041D cmp [0xEDF58], 0; je 0x426C1 */
        return code;                                                /* -100 */
    if (S16(REPLAYING) != 0) {                                      /* 0x4042A mov si, [0x107692]; jne 0x3FD35 */
        code = -1;                                                  /* 0x3FD35: ends the replay's click wait below */
        return code;
    }

    h = S16(0xBB400u);                                              /* 0x4043A dword [0xBB3FE] sar 16 */
    y = S16(0xBB3FCu);                                              /* 0x40445 dword [0xBB3FA] sar 16 */
    setD16(0xBB3FCu, 0x14C);                                        /* 0x4045A (di = 0x14C) */
    setD16(0xBB400u, 0x7B);                                         /* 0x40461 (ax = 0x7B) */
    f78884();                                                       /* 0x40467 the pointer hidden */
    w = S16(0xBB3FEu);                                              /* 0x40478 dword [0xBB3FC] sar 16 */
    x = S16(0xBB3FAu);                                              /* 0x40471 / 0x40491 dword [0xBB3F8] sar 16 */
    stack_makegif(x, w + x - 1, 0x14C, 0x1C6, &s_junk);            /* 0x4049A fg_makegif(x, x + w - 1, 0x14C, 0x1C6,
                                                                       "junk"): the message strip of the screen */
    f788AC();                                                       /* 0x404A2 */
    f3C828(PANEL_DLG);                                              /* 0x404AC every panel button disabled */
    snprintf((char *)DP(LABEL), 8, DSTR(0xB401Bu), (int)S32(0xEDF5Cu));   /* 0x404C9 sprintf(0x109E7C, "%d", round) */
    f3C284(PANEL_DLG, 0x101, LABEL);                                /* 0x404DB REPLAY's label (ebx = 0x109E7C, 0x404C4) */
    f3C314(PANEL_DLG, 0x101);                                       /* 0x404EA REPLAY */
    f3C314(PANEL_DLG, 0x24);                                        /* 0x404F9 TOGGLE UNITS */
    f3C314(PANEL_DLG, 0x23);                                        /* 0x40508 the mini-map */
    f3C314(PANEL_DLG, 0x1C);                                        /* 0x40517 scroll up */
    f3C314(PANEL_DLG, 0x1F);                                        /* 0x40526 scroll right */
    f3C314(PANEL_DLG, 0x21);                                        /* 0x40535 scroll down */
    f3C314(PANEL_DLG, 0x1E);                                        /* 0x40544 scroll left */
    snprintf((char *)DP(MSG), 0x1F4, DSTR(0xB401Eu), (int)S32(0xEDF5Cu));   /* 0x4055C sprintf(0xF5500, "Map shows
                                                                       Round %d post movement state.") */
    f3A990();                                                       /* 0x40564 erase the message strip */
    f3AB7C(DSTR(MSG));                                              /* 0x40573 (edx = 1, 0x4056E) */
    setD16(REPLAYING, 1);                                           /* 0x40578 mov [0x107692], dx */

    allocated = 0;                                                  /* 0x40581 */
    if (D32(SLOT_AREA) == 0) {                                      /* 0x40584 cmp [0xF5240], 0; jne 0x4059A */
        f7A8D0();                                                   /* 0x40592 */
        allocated = 1;                                              /* 0x40597 (ebx = 1) */
    }
    f7A940();                                                       /* 0x405A9 push the present state */
    if (g_original) {
        /* BUG(1995) 2: the record-0 bases: area records 0..159 and nation records 0..10, one off the stack's 1..160
           and 1..11 (file comment) */
        memcpy(DP(0xE31B8u), DP(0xE5D20u), SZ_AREA);                /* 0x4059A..0x405BB */
    } else {
        memcpy(DP(AREA1), DP(0xE5D20u + 0x45u), SZ_AREA);           /* records 1..160 */
    }
    memcpy(DP(TSTATE), DP(0xE2DF8u), SZ_TST);                       /* 0x405BE..0x405DA */
    if (g_original)
        memcpy(DP(0xF14CCu), DP(0xF1FB0u), SZ_NAT);                 /* 0x405DD..0x405FB (BUG(1995) 2) */
    else
        memcpy(DP(NAT1), DP(0xF1FB0u + 0xC7u), SZ_NAT);             /* records 1..11 */
    f3D52C(0, 0);                                                   /* 0x40600 (edx = 0, 0x405EC) */
    f3CBBC();                                                       /* 0x40605 */

    do
        code = f78BA4(PANEL_DLG, 0);                                /* 0x40611 */
    while (code != -1);                                             /* 0x4061C cmp ecx, -1; jne 0x4060A */

    f7AC60();                                                       /* 0x40623 pop: the present state back */
    setD16(REPLAYING, 0);                                           /* 0x40628 (ebx = 0, 0x40621) */
    if (allocated != 0)                                             /* 0x4062F */
        f7ABD0();                                                   /* 0x40635 */
    snprintf((char *)DP(LABEL), 8, DSTR(0xB4046u), (int)S16(0xF40FCu));   /* 0x40652 sprintf(0x109E7C, "%d", dword
                                                                       [0xF40FA] sar 16): this round */
    f3C284(PANEL_DLG, 0x101, LABEL);                                /* 0x40664 */
    f3C7A0(PANEL_DLG);                                              /* 0x4066E the enables as before */
    setD16(0xBB400u, (uint16_t)h);                                  /* 0x40676 */
    setD16(0xBB3FCu, (uint16_t)y);                                  /* 0x4067F */
    f3D52C(0, 0);                                                   /* 0x40689 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x40695 */
    fg_move(S16(0xBB3FAu), 0x14C);                                  /* 0x406AB dword [0xBB3F8] sar 16 */
    stack_showgif(&s_junk);                                         /* 0x406BA fg_showgif("junk", 3) */
    fg_vbclose();                                                   /* 0x406C2 */
    f3CBBC();                                                       /* 0x406C7 */
    code = -100;                                                    /* 0x406CC */
    return code;                                                    /* 0x406D3 */
}
