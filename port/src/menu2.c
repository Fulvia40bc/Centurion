/* Module MENU2 of docs/MAP_PLAN.md: the AUDIO/VISUALS preferences box 0x43E88, the "Scoring History" graph 0x44D5C,
   the nation's map view behind panel button 8 (0x44A34), the WAV loader 0x7B504, and the bodies of the dispatcher's
   cases 8 (0x3FC4B), 0x56 (0x3FD07) and 0xE2 (0x3FD49) as dcase_8 / dcase_56 / dcase_E2. Literal translations; the
   address of each instruction a line comes from is in its comment. D = read in the code, G = inference.
   Prototypes: menu2.h.

   Watcom runtime -> standard C:
     0x8C897 itoa(value, buf, 10) -> snprintf "%d" into a buffer of the original's size (100 bytes at [ebp-0xD0] in
             0x44D5C, shared with the sprintf below); radix 10 writes '-' and the magnitude, as "%d" does
     0x7DB3D sprintf -> snprintf bounded to the destination: [ebp-0xD0] (100 bytes) in 0x44D5C, [0xF5500] (500 bytes,
             as map.c and load.c) in 0x7B504
     0x7D8C8 fopen, 0x912D5 fwrite(buf eax, size edx, n ebx, fp ecx), 0x7D95E fclose -> the same (aciv.cfg, mode "wb"
             at 0xB4574, relative to the current directory as the original, which runs in the game folder; G: the port
             does too, save.c)
     repne scasb / not / dec (inline strlen) -> strlen; the inline 2-bytes-a-step copy 0x7B6C1 -> strcpy
     0x822B5 malloc -> load_heap_alloc (the block's pointer lives in a data field, game.h's handle scheme)
   __CHK dropped. The debug log 0x78B0C ("WAV File loaded", 0x7B843) is a no-op and left out. Library calls through
   fg.h in the original argument order (Fastgraph is cdecl: the pushes read right to left).

   AIL 3.03's file and memory helpers used by 0x7B504 and 0x43E88 (not game code; bodies D, game.h's music section):
   0x7D2E8 file size, 0x7D334 read a whole file, 0x7CB9D free a locked block, 0x7CCFD lock a range. newgame.c has them
   as static functions bound to its own block [0xF626C] (and saveload.c a static copy of 0x7D2E8), so this file keeps
   its own static copies, bound to load_heap handles (the sample blocks [0xF6260 + 4s]). A shared AIL helper file
   would merge the three sets (G: Claude's call; dispatch.c and newgame.c are not to be edited by this module).

   Miles AIL calls (port no-ops, marked "AIL (port no-op)"): the port plays its own sound (OVERVIEW §3, §7.2) and the
   sample voices are not wired to the game yet. Each call keeps the data reads of its arguments (the handles in
   [0xF6274] (DIG driver), [0xF6278 + 4s] (sample s), [0xF6284] (the sequence)); none of these calls writes the data
   object in the original either (AIL's own state lies outside the game's globals or in by-design ranges). Results:
     AIL_sample_status (0x843E0) -> 2 (SMP_DONE: nothing plays), AIL_active_sample_count (0x85398) -> 0,
     AIL_sequence_status (0x85C35) -> 2 (SEQ_DONE), AIL_set_sample_file (0x83E4A) -> 1 when a block is given, 0 without
     one (G: AIL fails on a missing file; that keeps the original's "Effects file" quit for a WAV that cannot be read).
   The others return nothing anyone reads: AIL_end_sample 0x8419B, AIL_set_sample_volume 0x8427E, AIL_start_sample
   0x84054, AIL_set_sample_loop_count 0x8436A, AIL_stop_sequence 0x85974, AIL_resume_sequence 0x859E1,
   AIL_set_sequence_loop_count 0x85BBF, AIL_set_sequence_volume 0x85B3D.
   How the port's sound would plug in (G, port design): AIL_set_sample_file(slot s, block) becomes wav_decode of the
   block (load_heap_ptr([0xF6260 + 4s], size)) into a wav_sound kept per slot; AIL_start_sample -> mix_play(mixer,
   &slot, volume, pan) remembering the voice; AIL_end_sample -> mix_stop(voice); AIL_sample_status -> mix_playing(voice)
   ? 4 : 2; AIL_active_sample_count -> the number of playing slot voices; AIL_set_sample_volume -> the volume passed to
   the next mix_play (0..127 as AIL's). The sequence calls go to the music player behind mix_set_music (xmiseq +
   gm_synth / opl): stop = pause, resume = continue, status 8 while paused, set_sequence_volume -> the music volume.
   Only these static stubs change; the translated code stays as it is.

   Data (D, read in the code at the addresses given):
     0xF097C  uint8   settings flags: 0x01 sound effects, 0x02 music, 0x04 animations (aciv.cfg byte 0)
     0xF097E  uint16  volumes 0..9: music in bits 0..6 (word & 0x7F), effects in bits 7..13 ((word << 2 as 16 bits)
                      >> 9). AIL volume = v * 127 / 9 (signed idiv). The inventory's "byte [0xF097F] & 0x7F" for
                      0x7B504 is wrong: 0x7B7F0 reads the effects field as above.
     0xF0980  6 bytes from 0xF097C are aciv.cfg (fwrite(0xF097C, 6, 1), 0x447E7)
     0xF40AE  int16   1 = too little XMS memory (main 0x100E7, "...we will have to disable the animations and sound
                      effects..."): the effects and animation switches and the effects volume do nothing then; music
                      stays switchable
     0xF625C  int8    digital sound on; 0xF625D int8 music on (title.c's boot_sound)
     0xF6118 + 64s   char[64] the WAV name in sample slot s (0..2; 0xF61D8 = slot 3 is the music's XMI name)
     0xF6260 + 4s    the slot's block (a heap pointer: a load_heap handle in the port; 0xF626C = slot 3 is the XMI's)
     0xF6278 + 4s    AIL sample handles (port: 1, title.c)
     0xF549C  char[]  the test sample's name: sprintf(0xF549C, "volcano.wav") at main 0x10757
     0xC2234  dialog, 8 records (obj2): 0x69 effects row (220, 91, 100x9), 0xDF effects '-' (327, 90, 11x11), 0xDE
              effects '+' (353, 90), 0x6A music row (220, 111), 0xE1 music '-' (327, 110), 0xE0 music '+' (353, 110),
              0x6B animations row (220, 131, 200x9), all style 2 (no label); 0x2A CONTINUE (265, 150, 110x19). The
              dispatcher's constant cases give 0x69/0x6A/0x6B -> -0x7B/-0x7C/-0x7D and 0xDE..0xE1 -> -0x83..-0x86.
     Sprites (title.c's 0x118F8): 0xF51DC ticked box, 0xF51E4 empty box (9x9), 0xF51BC / 0xF51C4 the '-' / '+' arrows
              (5x10)
     0xEE9F0  int16[10][45] score history, row n (1..9) column r (game.h); 0xF40FC int16 the round; byte [0xF4102]
              the phase (0x14 = 20: the round is one ahead there, G: after the AST phase's increment)
     0xC2838  int16[10] the nations' line colours (read as dword [0xC2836 + 2n] sar 16; obj2: 0, 63, 32, 68, 148, 167,
              11, 23, 18, 179), 0xC284C int16[10] their dash patterns (dword [0xC284A + 2n] sar 16; 0, 0xFFFF solid for
              AFRICA, CRETE, ASSYRIA, 0xF0F0 dashed for the others). The inventory's 0xC2836 / 0xC284A are the
              instructions' displacements, not the tables.
     0xBB800  dialog, 1 record: 0x2A CONTINUE (obj2 at (265, 450)); 0x44D5C moves it to (265, 455) and leaves it
              there; 0x44A34 appends it to the panel dialog 0xF596C and moves that copy to (265, 425)
     0xC197C[3] = "egypt5.gif", the background 0x44A34 shows; byte [0xEE168] the background of the map screen
     UI_CUR_DIALOG 0xF8E78: 0x43E88 and 0x44A34 save and restore it around their dialog; 0x44D5C does not (it stays
              0xBB800 afterwards)

   Heap pointers in data fields: [0xF6260 + 4s] (sample blocks, written by 0x7B504, freed by 0x7B504 and 0x43E88):
   load_heap handles (by design different from a DOSBox-X dump). As in the original the freed field keeps its stale
   value; the cleared name guards every later free (0x7B56E, 0x7B659, 0x4475A). AIL's file error [0xCAC68] is written
   by the copies of 0x7D2E8 / 0x7D334 as in the original (by design range, game.h).

   The "junk" GIF (0x44A34): the original saves the whole screen with fg_makegif(0, 639, 0, 479, "junk") only to read
   it back at the end with fg_move(0, 0) + fg_showgif("junk", 3) into the back buffer (flags 3: at the cursor, palette
   kept). GIF87a with 8-bit LZW is lossless and the palette is not loaded, so the pair copies pixels. fg.c has no
   fg_makegif; the port keeps the rectangle in memory (junk_save / junk_show below) and writes no file (MAP_PLAN risk
   7: default "not"). junk_show puts the pixels into the active virtual buffer at the cursor, opaque, clipped only on
   the right and bottom as fg_showgif (G: the surface is the 640x480 back buffer here), through the screen and
   fg_vbcut, putting the screen's bytes back afterwards, so the screen, the cursor, the colour, the palette and the
   clip region end up as with the GIF.

   1995 bugs (fixed unless g_original):
     - 0x44D5C stores the current score at column r of the 45-column history without the clamp to 44 that the AST
       phase uses (0x13BC7, game.h): from round 45 on it writes into the next nation's row, and for EGYPT (n = 9) past
       the table into [0xEED74] (0xEE9F0 + 90 * 9 + 2 * 45 = 0xEED74), a heap pointer that the dispatcher's NEW GAME /
       RESTORE cases free (D for the addresses; G: how often a game reaches round 45). Fixed: the column is clamped to
       44, the column the AST phase writes from round 44 on.
     - 0x44D5C draws "R O U N D S" in whatever colour the round labels left: 15, except with 37..44 rounds and an odd
       count, where the last label is skipped and the text comes out in the grid's colour 7 (D; G: unintended). Fixed:
       colour 15 is set first.
   Oddities kept (D):
     - 0x7B504 runs its "free the old block" check three more times in a loop (0x7B5EE..0x7B6AC), each time on slot s
       (not on the loop index) and only after the name was already cleared, so the loop never frees anything.
     - 0x43E88's exit sets the volume of sample slot 0 ([0xF6278]) while the volume test played slot 1 ([0xF627C]).
     - 0x44D5C's tick marks on the score axis are 6 px long, 4 px at 500-steps (k = 5, 10: fg_draw to x 0x29, not 0x27).
     - 0x44D5C divides by the round count: with round 0 (or round 1 in phase 0x14) the original takes a divide error
       (G: never; rounds start at 1). Port guard: a step of 0 there (x86 would trap, ARM64 gives 0).
     - 0x44A34 leaves panel button 8 (its own) enabled, so the map view can be opened again from inside itself; the
       inner call then saves the map view over the outer "junk" (one buffer, as the one file), and the outer exit
       shows the map view instead of the screen before it (G: whether the panel dialog holds an enabled id 8 then).
       Kept as the original. */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fg.h"
#include "game.h"
#include "map.h"
#include "mapdata.h"
#include "menu2.h"
#include "panel.h"
#include "save.h"
#include "state.h"
#include "ui.h"

#define BACKBUF   0xF52C8u   /* int32 virtual-buffer handle of the back buffer */
#define BGBUF     0xF52CCu   /* int32 handle of the background-picture buffer */
#define PANEL_DLG 0xF596Cu   /* the map screen's panel dialog */
#define DLG_CONT  0xBB800u   /* CONTINUE (0x2A) */
#define DLG_PREFS 0xC2234u   /* the AUDIO/VISUALS dialog */
#define CFG_FLAGS 0xF097Cu   /* uint8: 0x01 effects, 0x02 music, 0x04 animations */
#define CFG_VOLS  0xF097Eu   /* uint16: music bits 0..6, effects bits 7..13 */
#define SPR_ON    0xF51DCu   /* the ticked box */
#define SPR_OFF   0xF51E4u   /* the empty box */
#define SPR_MINUS 0xF51BCu
#define SPR_PLUS  0xF51C4u
#define SLOT_NAME(s)   (0xF6118u + 64u * (uint32_t)(s))   /* shl 6; add 0xF6118 */
#define SLOT_BLOCK(s)  (0xF6260u + 4u * (uint32_t)(s))
#define SLOT_SAMPLE(s) (0xF6278u + 4u * (uint32_t)(s))

/* the effects volume: mov ax, word [0xF097E]; shl eax, 2; shr ax, 9 (bits 7..13) */
static uint32_t eff_volume(void)
{
    return (uint32_t)(uint16_t)(D16(CFG_VOLS) << 2) >> 9;
}

/* the music volume: mov ax, word [0xF097E]; xor ah, ah; and al, 0x7F */
static uint32_t mus_volume(void)
{
    return (uint32_t)(D16(CFG_VOLS) & 0x7Fu);
}

/* v * 127 / 9: mov eax, edx; shl eax, 7; sub eax, edx; cdq (sar edx, 0x1F); idiv 9 */
static int32_t ail_volume(uint32_t v)
{
    return (int32_t)(v * 127u) / 9;
}

/* ---- Miles AIL 3.03: port no-ops (file comment) ----------------------------------------------------------------- */

static int32_t ail_sample_status(uint32_t sample)          /* AIL (port no-op) 0x843E0 */
{
    (void)sample;
    return 2;                                               /* SMP_DONE: no sample plays in the port */
}

static void ail_end_sample(uint32_t sample)                 /* AIL (port no-op) 0x8419B */
{
    (void)sample;
}

static int32_t ail_active_sample_count(uint32_t dig)       /* AIL (port no-op) 0x85398 */
{
    (void)dig;
    return 0;
}

static void ail_set_sample_volume(uint32_t sample, int32_t volume)   /* AIL (port no-op) 0x8427E */
{
    (void)sample;
    (void)volume;
}

static void ail_start_sample(uint32_t sample)               /* AIL (port no-op) 0x84054 */
{
    (void)sample;
}

static int32_t ail_set_sample_file(uint32_t sample, uint32_t block, int32_t block_type)   /* AIL (port no-op) 0x83E4A */
{
    (void)sample;
    (void)block_type;
    return block != 0 ? 1 : 0;                              /* G: AIL fails without a file image */
}

static void ail_set_sample_loop_count(uint32_t sample, int32_t count)   /* AIL (port no-op) 0x8436A */
{
    (void)sample;
    (void)count;
}

static void ail_stop_sequence(uint32_t seq)                 /* AIL (port no-op) 0x85974 */
{
    (void)seq;
}

static int32_t ail_sequence_status(uint32_t seq)           /* AIL (port no-op) 0x85C35 */
{
    (void)seq;
    return 2;                                               /* SEQ_DONE (G: the port's music is not wired here) */
}

static void ail_resume_sequence(uint32_t seq)               /* AIL (port no-op) 0x859E1 */
{
    (void)seq;
}

static void ail_set_sequence_loop_count(uint32_t seq, int32_t count)   /* AIL (port no-op) 0x85BBF */
{
    (void)seq;
    (void)count;
}

static void ail_set_sequence_volume(uint32_t seq, int32_t volume, int32_t ms)   /* AIL (port no-op) 0x85B3D */
{
    (void)seq;
    (void)volume;
    (void)ms;
}

/* ---- AIL 3.03's file helpers (static copies, file comment) ------------------------------------------------------ */

/* 0x7D2E8: AIL's file size (as newgame.c's static f7D2E8): [0xCAC68] = 0; open the file (DOS open 0x93F92, mode 0x200:
   read, binary, G); if it does not open, [0xCAC68] = 3 and -1; else filelength (0x941E5; -1 -> [0xCAC68] = 5), close
   (0x941E0), the length. cdecl, one stack argument. */
static long f7D2E8(const char *name)
{
    FILE *f;
    long size;

    setD32(0xCAC68u, 0);                              /* 0x7D2EA */
    f = fopen(name, "rb");                            /* 0x7D2FA */
    if (f == NULL) {                                  /* 0x7D304 cmp eax,-1 */
        setD32(0xCAC68u, 3);                          /* 0x7D309 */
        return -1;                                    /* 0x7D313 */
    }
    size = -1;
    if (fseek(f, 0, SEEK_END) == 0)                   /* 0x7D314 filelength */
        size = ftell(f);
    if (size == -1)                                   /* 0x7D31B */
        setD32(0xCAC68u, 5);                          /* 0x7D320 */
    fclose(f);                                        /* 0x7D32C */
    return size;                                      /* 0x7D331 */
}

/* 0x7D334: AIL's file read with dest = 0 (the only form used here, 0x7B75E push 0): the whole file into a new block
   from the allocation hook [0xCAC6C] (malloc; port: a load_heap handle); returns it, or 0 with [0xCAC68] = 3 (no
   file), 2 (no memory) or 5 (short read), freeing the block (hook [0xCAC70]) on the error paths. cdecl. */
static uint32_t f7D334(const char *name)
{
    long size;                                        /* edi */
    uint32_t block;                                   /* ecx */
    uint8_t *p;
    FILE *f;                                          /* esi */

    setD32(0xCAC68u, 0);                              /* 0x7D338 */
    size = f7D2E8(name);                              /* 0x7D343 */
    if (size == -1) {                                 /* 0x7D34D */
        setD32(0xCAC68u, 3);                          /* 0x7D352 */
        return 0;                                     /* 0x7D35C */
    }
    block = load_heap_alloc((uint32_t)size);          /* 0x7D369 call [0xCAC6C] (0x822B5) with eax = size */
    if (block == 0) {                                 /* 0x7D371 */
        setD32(0xCAC68u, 2);                          /* 0x7D375 */
        return 0;
    }
    p = load_heap_ptr(block, (uint32_t)size);
    f = fopen(name, "rb");                            /* 0x7D38B open(name, 0x200) */
    if (f == NULL) {                                  /* 0x7D395 */
        load_heap_free(block);                        /* 0x7D39C call [0xCAC70] */
        setD32(0xCAC68u, 3);                          /* 0x7D352 */
        return 0;
    }
    if (p == NULL || (long)fread(p, 1, (size_t)size, f) != size) {   /* 0x7D3A8 read; 0x7D3AD cmp eax,edi */
        load_heap_free(block);                        /* 0x7D3B3 */
        setD32(0xCAC68u, 5);                          /* 0x7D3B9 */
        fclose(f);   /* port: the original leaves the DOS handle open here (0x7D3C3 jmp 0x7D35C) */
        return 0;
    }
    fclose(f);                                        /* 0x7D3C7 */
    return block;                                     /* 0x7D3CC */
}

/* 0x7CB9D: AIL's free of a locked block: if p, unlock p..p+size (0x7CD14, DPMI, no data written) and free it through
   the hook [0xCAC70]. cdecl, stack: p, size. The port's p is a load_heap handle. */
static void f7CB9D(uint32_t handle, long size)
{
    (void)size;                                       /* the unlock's range: nothing to unlock in the port */
    if (handle != 0)                                  /* 0x7CBA2 test esi,esi */
        load_heap_free(handle);                       /* 0x7CBB6 call [0xCAC70] */
}

/* 0x7CCFD: AIL's lock of p..p+size (0x7CC6F: DPMI 0600h, no data written). Nothing to lock in the port. */
static void f7CCFD(uint32_t handle, long size)
{
    (void)handle;
    (void)size;
}

/* ---- The "junk" screen save (port stand-in for fg_makegif 0x8CE50 + fg_showgif 0x88F98, file comment) ---------- */

#define SCR_W 640
#define SCR_H 480

static uint8_t s_junk[SCR_W * SCR_H];   /* the saved rectangle, row by row */
static int s_junk_w, s_junk_h;          /* 0 = nothing saved (fg_showgif would find no file: returns 1) */
static uint8_t s_junk_scr[SCR_W * SCR_H];

/* fg_makegif(minx, maxx, miny, maxy, name) of the screen (no virtual buffer is open at the call site, D) */
static int junk_save(int minx, int maxx, int miny, int maxy, const char *name)
{
    const uint8_t *scr = fg_screen();
    int r;

    (void)name;                                       /* "junk" */
    if (minx < 0 || miny < 0 || maxx >= SCR_W || maxy >= SCR_H || maxx < minx || maxy < miny)
        return 0;                                     /* port guard: only rectangles inside the screen */
    s_junk_w = maxx - minx + 1;
    s_junk_h = maxy - miny + 1;
    for (r = 0; r < s_junk_h; r++)
        memcpy(s_junk + (size_t)r * (size_t)s_junk_w, scr + (size_t)(miny + r) * SCR_W + (size_t)minx,
               (size_t)s_junk_w);
    return 0;
}

/* fg_showgif(name, flags) for the saved rectangle with flags 3 (at the cursor, palette kept) into the active virtual
   buffer: min(x + w - 1, 639) - x + 1 columns and as many rows, opaque. The pixels go through the screen rectangle at
   the cursor and fg_vbcut (screen -> the last opened virtual buffer, no clipping); the screen's own bytes are put back
   afterwards. */
static int junk_show(const char *name, int flags)
{
    uint8_t *scr = fg_screen();
    int x0 = fg_getxpos(), y0 = fg_getypos();         /* flags & 2: the graphics cursor (0x8927D) */
    int w, rows, r;

    (void)name;                                       /* "junk" */
    (void)flags;                                      /* 3 at the only call site */
    if (s_junk_w == 0)
        return 1;                                     /* no file */
    w = (x0 + s_junk_w - 1 < SCR_W - 1 ? x0 + s_junk_w - 1 : SCR_W - 1) - x0 + 1;
    rows = (y0 + s_junk_h - 1 < SCR_H - 1 ? y0 + s_junk_h - 1 : SCR_H - 1) - y0 + 1;
    if (x0 < 0 || y0 < 0 || w <= 0 || rows <= 0)
        return 0;                                     /* port guard (the original does not check negative x, y) */
    for (r = 0; r < rows; r++) {
        uint8_t *line = scr + (size_t)(y0 + r) * SCR_W + (size_t)x0;
        memcpy(s_junk_scr + (size_t)r * (size_t)w, line, (size_t)w);
        memcpy(line, s_junk + (size_t)r * (size_t)s_junk_w, (size_t)w);
    }
    fg_vbcut(x0, x0 + w - 1, y0, y0 + rows - 1, x0, y0 + rows - 1);
    for (r = 0; r < rows; r++)
        memcpy(scr + (size_t)(y0 + r) * SCR_W + (size_t)x0, s_junk_scr + (size_t)r * (size_t)w, (size_t)w);
    return 0;
}

/* ---- 0x7B504: load a WAV into sample slot s ---------------------------------------------------------------------- */

/* the block of slot s freed with its size: cat_size again when the name is in ACIV.CT5, else the file's size
   (0x7B574..0x7B5D8 and its copy 0x7B65B..0x7B6AC) */
static void slot_free(int s)
{
    long size;                                        /* [ebp-0x14] */
    long arg;

    size = cat_size(DSTR(SLOT_NAME(s)), S32(0xF40E4u));   /* 0x7B587 cat_size(name, [0xF40E4] = ACIV.CT5) */
    if (size != -2)                                   /* 0x7B5A0 cmp esi, -2 */
        arg = cat_size(DSTR(SLOT_NAME(s)), S32(0xF40E4u));   /* 0x7B5AD the second lookup */
    else
        arg = f7D2E8(DSTR(SLOT_NAME(s)));             /* 0x7B5C4 */
    f7CB9D(D32(SLOT_BLOCK(s)), arg);                  /* 0x7B5D8 */
}

void f7B504(int s, const char *name)
{
    int32_t i;                                        /* [ebp-0x18] */
    long size;                                        /* [ebp-0x14] */
    uint8_t *p;

    if (D8(0xF625Cu) == 0)                            /* 0x7B51E: digital sound off */
        return;
    if (ail_sample_status(D32(SLOT_SAMPLE(s))) != 2)  /* 0x7B536; 0x7B53E cmp eax, 2 (SMP_DONE) */
        ail_end_sample(D32(SLOT_SAMPLE(s)));          /* 0x7B54E */
    if (strlen(DSTR(SLOT_NAME(s))) != 0) {            /* 0x7B567 repne scasb; 0x7B56E je 0x7B5EE */
        slot_free(s);                                 /* 0x7B574..0x7B5DD */
        setD8(SLOT_NAME(s), 0);                       /* 0x7B5E8 */
    }
    for (i = 0; i < 3; i++) {                         /* 0x7B5EE; 0x7B626 cmp 3; jge 0x7B6B1 */
        if (ail_sample_status(D32(SLOT_SAMPLE(s))) != 2)   /* 0x7B63B: slot s, not i (D) */
            continue;                                 /* 0x7B646 jne 0x7B620 */
        if (S8(SLOT_NAME(s)) == 0)                    /* 0x7B64E dword [0xF6115 + 64s] sar 24; je 0x7B620 */
            continue;
        slot_free(s);                                 /* 0x7B65B..0x7B60F */
        setD8(SLOT_NAME(s), 0);                       /* 0x7B61A */
    }
    strcpy((char *)DP(SLOT_NAME(s)), name);           /* 0x7B6C1..0x7B6D7 */
    if (strlen(DSTR(SLOT_NAME(s))) == 0)              /* 0x7B6EB; 0x7B6F2 je 0x7B848 */
        return;
    size = cat_size(DSTR(SLOT_NAME(s)), S32(0xF40E4u));   /* 0x7B70B */
    if (size != -2) {                                 /* 0x7B719 cmp ecx, -2 */
        setD32(SLOT_BLOCK(s), load_heap_alloc((uint32_t)size));   /* 0x7B721 malloc(size); 0x7B72F */
        p = load_heap_ptr(D32(SLOT_BLOCK(s)), (uint32_t)size);
        if (p != NULL)   /* port guard: the original hands cat_load a null buffer when the malloc fails */
            (void)cat_load(DSTR(SLOT_NAME(s)), p, S32(0xF40E4u));   /* 0x7B74E; the result is not checked */
    } else {                                          /* 0x7B758: not in the catalogue: the loose file */
        setD32(SLOT_BLOCK(s), f7D334(DSTR(SLOT_NAME(s))));   /* 0x7B766 f7D334(name, 0); 0x7B771 */
    }
    f7CCFD(D32(SLOT_BLOCK(s)), f7D2E8(DSTR(SLOT_NAME(s))));   /* 0x7B784, 0x7B798: lock the block */
    if (ail_set_sample_file(D32(SLOT_SAMPLE(s)), D32(SLOT_BLOCK(s)), 0) == 0) {   /* 0x7B7B8; 0x7B7C2 jne */
        snprintf((char *)DP(0xF5500u), 0x1F4, DSTR(0xB971Bu),   /* 0x7B7E3 "%s\nEffects file:%s,(%d) \n" */
                 DSTR(0x11AF7Cu), DSTR(SLOT_NAME(s)), s);       /* AIL's error text, the name, the slot */
        f44CAC();                                     /* 0x7B7EB */
    }
    ail_set_sample_volume(D32(SLOT_SAMPLE(s)), ail_volume(eff_volume()));   /* 0x7B7F0..0x7B821 */
    ail_set_sample_loop_count(D32(SLOT_SAMPLE(s)), 1);   /* 0x7B836 */
    /* 0x7B843: the debug log "WAV File loaded" (0x78B0C, a no-op) */
}

/* ---- 0x43E88: AUDIO/VISUALS -------------------------------------------------------------------------------------- */

/* one sunken volume box with its number, on the active surface: f3C9D0(0x154, y - 1, 0xB, 0xC, 5), fg_setcolor(5),
   f3A148(0x155, y, 0xB, v) (0x43F79..0x43FAE and its copies) */
static void volume_box(int y, uint32_t v)
{
    f3C9D0(0x154, y - 1, 0xB, 0xC, 5);
    fg_setcolor(5);
    f3A148(0x155, y, 0xB, (int)v);
}

void f43E88(void)
{
    int32_t changed;                                  /* [ebp-0x20] */
    uint32_t saved_dlg;                               /* [ebp-0x1C] */
    int32_t r;                                        /* [ebp-0x24] */
    FILE *fp;                                         /* [ebp-0x18] */
    uint8_t f;
    uint16_t w;
    uint32_t v;

    changed = 0;                                      /* 0x43E9F */
    fg_vbopen(S32(BACKBUF));                          /* 0x43EA9 */
    f3C9D0(0xD2, 0x50, 0xDC, 0x64, 3);                /* 0x43EC7: filled, raised */
    fg_setcolor(5);                                   /* 0x43ECE */
    f3A044(0xEB, 0x5A, DSTR(0xB4547u));               /* 0x43EE5 "Sound effects" */
    f3A044(0x16D, 0x5A, DSTR(0xB4555u));              /* 0x43EF9 "Volume" */
    f3A044(0xEB, 0x6E, DSTR(0xB455Cu));               /* 0x43F0D "Music" */
    f3A044(0x16D, 0x6E, DSTR(0xB4562u));              /* 0x43F21 "Volume" */
    f3A044(0xEB, 0x82, DSTR(0xB4569u));               /* 0x43F35 "Animations" */
    f3AF8C((D8(CFG_FLAGS) & 0x01) ? SPR_ON : SPR_OFF, 0xDC, 0x5B);   /* 0x43F3A; 0x43F63 */
    volume_box(0x5A, eff_volume());                   /* 0x43F7E, 0x43F85, 0x43FAE */
    f3AF8C(SPR_MINUS, 0x14D, 0x5A);                   /* 0x43FC2 */
    f3AF8C(SPR_PLUS, 0x161, 0x5A);                    /* 0x43FD6 */
    f3AF8C((D8(CFG_FLAGS) & 0x02) ? SPR_ON : SPR_OFF, 0xDC, 0x6F);   /* 0x43FDB; 0x44004 */
    volume_box(0x6E, mus_volume());                   /* 0x4401F, 0x44026, 0x4404C */
    f3AF8C(SPR_MINUS, 0x14D, 0x6E);                   /* 0x44060 */
    f3AF8C(SPR_PLUS, 0x161, 0x6E);                    /* 0x44074 */
    f3AF8C((D8(CFG_FLAGS) & 0x04) ? SPR_ON : SPR_OFF, 0xDC, 0x83);   /* 0x44079; 0x440A2 */
    fg_vbclose();                                     /* 0x440A7 */
    saved_dlg = D32(UI_CUR_DIALOG);                   /* 0x440AC */
    f3B05C(DLG_PREFS);                                /* 0x440B9: its buttons into the back buffer */
    setD32(UI_CUR_DIALOG, saved_dlg);                 /* 0x440C1 */
    f78884();                                         /* 0x440C6 */
    fg_vbpaste(0xD2, 0x1AD, 0x50, 0xB3, 0xD2, 0xB3);  /* 0x440E6 */

unhide:
    f788AC();                                         /* 0x440EE */
wait:
    r = f78BA4(DLG_PREFS, 0);                         /* 0x440FA */
    if (r == -100)                                    /* 0x44105 */
        goto wait;
    if (r == -1)                                      /* 0x4410D CONTINUE */
        goto done;

    if (r == -0x7B && D16(0xF40AEu) == 0) {           /* 0x44119; 0x44122: sound effects on/off */
        changed = 1;                                  /* 0x44130 */
        f78884();                                     /* 0x44137 */
        f = D8(CFG_FLAGS);                            /* 0x4413C */
        if (f & 0x01) {                               /* 0x44142 */
            setD8(CFG_FLAGS, (uint8_t)(f & 0xFEu));   /* 0x4414E and bh, -2; 0x44156 */
            f3AF8C(SPR_OFF, 0xDC, 0x5B);              /* 0x44161 */
            if (D8(0xF625Cu) != 0                     /* 0x4416C */
                && ail_active_sample_count(D32(0xF6274u)) > 0)   /* 0x4417B; 0x44185 jle */
                ail_end_sample(D32(0xF6278u));        /* 0x44192: slot 0 */
        } else {
            setD8(CFG_FLAGS, (uint8_t)(f | 0x01u));   /* 0x441AB or ch, 1; 0x441B3 */
            f3AF8C(SPR_ON, 0xDC, 0x5B);               /* 0x441B9 */
        }
        goto unhide;
    }

    if (r == -0x84 && D16(0xF40AEu) == 0) {           /* 0x441C6; 0x441D2: effects volume - 1 */
        if ((D8(CFG_FLAGS) & 0x01) == 0)              /* 0x441E5 */
            goto wait;
        v = eff_volume();                             /* 0x441ED..0x441FC */
        if ((int32_t)v <= 0)                          /* 0x44201 jle */
            goto wait;
        changed = 1;                                  /* 0x44207 */
        w = D16(CFG_VOLS);                            /* 0x44214 */
        setD16(CFG_VOLS, (uint16_t)((w & 0xC07Fu) | (((v - 1u) & 0x7Fu) << 7)));   /* 0x4422A, 0x44238 */
        f78884();                                     /* 0x4423F */
        volume_box(0x5A, eff_volume());               /* 0x44250, 0x44257, 0x44280 */
        f788AC();                                     /* 0x44285 */
        f7B504(1, DSTR(0xF549Cu));                    /* 0x44294: the test sample ("volcano.wav") into slot 1 */
        if (D8(0xF625Cu) != 0) {                      /* 0x44299 */
            ail_set_sample_volume(D32(0xF627Cu), ail_volume(eff_volume()));   /* 0x442D4 */
            ail_start_sample(D32(0xF627Cu));          /* 0x442E2 */
        }
        goto wait;
    }

    if (r == -0x83 && D16(0xF40AEu) == 0) {           /* 0x442F2; 0x442FE: effects volume + 1 */
        if ((D8(CFG_FLAGS) & 0x01) == 0)              /* 0x44312 */
            goto wait;
        v = eff_volume();                             /* 0x4431B..0x4432A */
        if ((int32_t)v >= 9)                          /* 0x4432D cmp edx, 9; jge */
            goto wait;
        changed = 1;                                  /* 0x44336 */
        w = D16(CFG_VOLS);                            /* 0x44343 */
        setD16(CFG_VOLS, (uint16_t)((w & 0xC07Fu) | (((v + 1u) & 0x7Fu) << 7)));   /* 0x44359, 0x44367 */
        f78884();                                     /* 0x4436E */
        volume_box(0x5A, eff_volume());               /* 0x4437F, 0x44386, 0x443AF */
        f788AC();                                     /* 0x443B4 */
        f7B504(1, DSTR(0xF549Cu));                    /* 0x443C3 */
        if (D8(0xF625Cu) != 0) {                      /* 0x443C8 */
            ail_set_sample_volume(D32(0xF627Cu), ail_volume(eff_volume()));   /* 0x44403 */
            ail_start_sample(D32(0xF627Cu));          /* 0x44412 */
        }
        goto wait;
    }

    if (r == -0x7C) {                                 /* 0x44422: music on/off (no [0xF40AE] test) */
        changed = 1;                                  /* 0x4442B */
        f78884();                                     /* 0x44432 */
        f = D8(CFG_FLAGS);                            /* 0x44437 */
        if (f & 0x02) {                               /* 0x4443D */
            setD8(CFG_FLAGS, (uint8_t)(f & 0xFDu));   /* 0x44449 and bh, -3; 0x44451 */
            f3AF8C(SPR_OFF, 0xDC, 0x6F);              /* 0x4445C */
            if (D8(0xF625Du) != 0)                    /* 0x44467 */
                ail_stop_sequence(D32(0xF6284u));     /* 0x44476 */
        } else {
            setD8(CFG_FLAGS, (uint8_t)(f | 0x02u));   /* 0x4448F or ch, 2; 0x44497 */
            f3AF8C(SPR_ON, 0xDC, 0x6F);               /* 0x4449D */
            if (D8(0xF625Du) != 0                     /* 0x444A7 */
                && ail_sequence_status(D32(0xF6284u)) == 8) {   /* 0x444B6; 0x444BE cmp eax, 8 (SEQ_STOPPED) */
                ail_resume_sequence(D32(0xF6284u));   /* 0x444CE */
                ail_set_sequence_loop_count(D32(0xF6284u), 0);   /* 0x444DF: forever */
            }
        }
        goto unhide;
    }

    /* 0x444EC: dx = word & 0x7F (the music volume), ax = word & 0xFF80, read before the remaining compares */
    w = D16(CFG_VOLS);
    v = mus_volume();

    if (r == -0x86) {                                 /* 0x44503: music volume - 1 */
        if ((D8(CFG_FLAGS) & 0x02) == 0)              /* 0x44515 */
            goto wait;
        if ((int32_t)v <= 0)                          /* 0x44523 test ecx, ecx; jle */
            goto wait;
        changed = 1;                                  /* 0x4452B */
        setD16(CFG_VOLS, (uint16_t)((w & 0xFF80u) | ((v - 1u) & 0x7Fu)));   /* 0x4453D, 0x4454A */
        f78884();                                     /* 0x44550 */
        volume_box(0x6E, mus_volume());               /* 0x44561, 0x44568, 0x4458E */
        f788AC();                                     /* 0x44593 */
        if (D8(0xF625Du) != 0)                        /* 0x44598 */
            ail_set_sequence_volume(D32(0xF6284u), ail_volume(mus_volume()), 0);   /* 0x445D2 */
        goto wait;
    }

    if (r == -0x85) {                                 /* 0x445E2: music volume + 1 */
        if ((D8(CFG_FLAGS) & 0x02) == 0)              /* 0x445F4 */
            goto wait;
        if ((int32_t)v >= 9)                          /* 0x44602 cmp ecx, 9; jge */
            goto wait;
        changed = 1;                                  /* 0x4460B */
        setD16(CFG_VOLS, (uint16_t)((w & 0xFF80u) | ((v + 1u) & 0x7Fu)));   /* 0x4461D, 0x4462A */
        f78884();                                     /* 0x44630 */
        volume_box(0x6E, mus_volume());               /* 0x44641, 0x44648, 0x4466E */
        f788AC();                                     /* 0x44673 */
        if (D8(0xF625Du) != 0)                        /* 0x44678 */
            ail_set_sequence_volume(D32(0xF6284u), ail_volume(mus_volume()), 0);   /* 0x446B1 */
        goto wait;
    }

    if (r == -0x7D && D16(0xF40AEu) == 0) {           /* 0x446C1; 0x446CA: animations on/off */
        changed = 1;                                  /* 0x446D8 */
        f78884();                                     /* 0x446DF */
        f = D8(CFG_FLAGS);                            /* 0x446E4 */
        if (f & 0x04) {                               /* 0x446E9 */
            setD8(CFG_FLAGS, (uint8_t)(f & 0xFBu));   /* 0x446F4 and ah, -5; 0x446FC */
            f3AF8C(SPR_OFF, 0xDC, 0x83);              /* 0x44723 */
        } else {
            setD8(CFG_FLAGS, (uint8_t)(f | 0x04u));   /* 0x44710 or dl, 4; 0x44718 */
            f3AF8C(SPR_ON, 0xDC, 0x83);               /* 0x44723 */
        }
        goto unhide;                                  /* 0x44728 */
    }
    goto wait;                                        /* 0x446C4 / 0x446D2: any other code */

done:
    if (D8(0xF625Cu) != 0) {                          /* 0x4472D */
        ail_end_sample(D32(0xF627Cu));                /* 0x44741: the test sample's slot 1 */
        if (strlen(DSTR(0xF6158u)) != 0) {            /* 0x44753 repne scasb on slot 1's name; 0x4475A */
            long size = f7D2E8(DSTR(0xF6158u));       /* 0x44761 */
            f7CB9D(D32(0xF6264u), size);              /* 0x44771: slot 1's block */
            setD8(0xF6158u, 0);                       /* 0x4477B */
        }
        ail_set_sample_volume(D32(0xF6278u), ail_volume(eff_volume()));   /* 0x44781..0x447AE: slot 0 */
    }
    if (changed == 1) {                               /* 0x447B6 */
        fp = fopen(DSTR(0xB4577u), DSTR(0xB4574u));   /* 0x447C6 fopen("aciv.cfg", "wb") */
        if (fp != NULL) {                             /* 0x447D1 */
            (void)fwrite(DP(CFG_FLAGS), 6, 1, fp);    /* 0x447E7 fwrite(0xF097C, 6, 1, fp) */
            fclose(fp);                               /* 0x447EF */
        }
    }
}

/* ---- 0x44D5C: Scoring History ------------------------------------------------------------------------------------ */

/* nation n's record field */
static uint32_t nat(int32_t n, uint32_t off)
{
    return GAME_NATION + 199u * (uint32_t)n + off;
}

/* int16 history entry of nation n, column k: 0xEE9F0 + 90n + 2k (lea 3n; add; shl 4; sub: 90n), read as
   dword [0xEE9EE + 90n + 2k] sar 16 */
static uint32_t hist(int32_t n, int32_t k)
{
    return 0xEE9F0u + (uint32_t)(90 * n + 2 * k);
}

void f44D5C(void)
{
    int32_t r;                                        /* [ebp-0x20] the round's column */
    int32_t n;                                        /* [ebp-0x1C] (the nation, the sort's bound, the loops' k) */
    int32_t k;                                        /* [ebp-0x4C] */
    int32_t maxs;                                     /* [ebp-0x34] the largest score in the history */
    int32_t cnt;                                      /* [ebp-0x3C] the pairs */
    int32_t ymax;                                     /* [ebp-0x30] the scale's top */
    int32_t gstep;                                    /* [ebp-0x28] the grid step, 36 px */
    int32_t rounds;                                   /* [ebp-0x44] */
    int32_t xstep;                                    /* [ebp-0x2C] */
    int32_t li;                                       /* [ebp-0x40] the legend line */
    int32_t nn;                                       /* [ebp-0x38] */
    int32_t y;                                        /* [ebp-0x24] */
    int32_t code;                                     /* [ebp-0x48] */
    int32_t s, col, v;
    int16_t pair[9][2];                               /* [ebp-0x6C]: {score, nation} dwords */
    int16_t t0, t1;                                   /* the swap's dword [ebp-0x18] */
    char buf[0x64];                                   /* [ebp-0xD0], up to the pairs */

    if (D8(0xF4102u) == 0x14)                         /* 0x44D74 */
        r = (int32_t)S16(0xF40FCu) - 1;               /* 0x44D7D dword [0xF40FA] sar 16; dec */
    else
        r = (int32_t)S16(0xF40FCu);                   /* 0x44D88 */
    f3CD10();                                         /* 0x44D95: screen -> background buffer */
    maxs = 0;                                         /* 0x44D9A (edx = 0, kept across the call) */
    cnt = 0;                                          /* 0x44D9D */
    for (n = 1; n <= 9; n++) {                        /* 0x44DA0; 0x44DAF cmp 9; jg */
        if (D8(nat(n, 0x9A)) == 0)                    /* 0x44DC0 byte [199n + 0xF1566]: not in play */
            continue;
        s = f147C4(n);                                /* 0x44DCD */
        col = r;
        if (!g_original && col > 44)
            col = 44;   /* BUG(1995): the store is not clamped to the 45 columns (file comment) */
        setD16(hist(n, col), (uint16_t)s);            /* 0x44DF2 mov word [90n + 2r + 0xEE9F0], bx */
        pair[cnt][0] = (int16_t)s;                    /* 0x44DFE word [ebp + 4cnt - 0x6C] */
        pair[cnt][1] = (int16_t)n;                    /* 0x44E13 word [ebp + 4cnt - 0x6A] */
        cnt++;                                        /* 0x44E06 */
        for (k = 0; k < 0x2D; k++) {                  /* 0x44E1A; 0x44E25 cmp 0x2D; jge */
            v = (int32_t)S16(hist(n, k));             /* 0x44E49 sar 0x10 */
            if (v > maxs)                             /* 0x44E56 cmp eax, edx; jle */
                maxs = v;                             /* 0x44E7E */
        }
    }
    ymax = ((maxs + 0x63) / 0x64) * 100;              /* 0x44E86..0x44EA4: idiv 100, then *100 by shifts */
    gstep = 0x24;                                     /* 0x44EA7 */
    rounds = r;                                       /* 0x44EB1 */
    if (rounds >= 0x2D)                               /* 0x44EB4 */
        rounds = 0x2C;                                /* 0x44EBA */
    if (rounds != 0)
        xstep = 0x212 / rounds;                       /* 0x44EC1..0x44ED0 cdq; idiv */
    else
        xstep = 0;   /* port guard: the original takes a divide error here (file comment) */

    /* bubble sort, highest score first (0x44ED3..0x44F41): signed 16-bit compare, the dwords swapped */
    for (n = cnt - 2; n >= 0; n--)                    /* 0x44ED6, 0x44EEC dec; jl 0x44F43 */
        for (k = 0; k <= n; k++)                      /* 0x44EE2, 0x44F05 cmp; jg */
            if (pair[k + 1][0] > pair[k][0]) {        /* 0x44F1A cmp ax, bx; jle */
                t0 = pair[k + 1][0];                  /* 0x44F22 */
                t1 = pair[k + 1][1];
                pair[k + 1][0] = pair[k][0];          /* 0x44F33 */
                pair[k + 1][1] = pair[k][1];
                pair[k][0] = t0;                      /* 0x44F3D */
                pair[k][1] = t1;
            }

    fg_vbopen(S32(BACKBUF));                          /* 0x44F4A */
    fg_setcolor(5);                                   /* 0x44F54 */
    fg_rect(0, 0x27F, 0, 0x1DF);                      /* 0x44F6A */
    f39F90(2);                                        /* 0x44F77 */
    fg_setcolor(4);                                   /* 0x44F7E */
    f3A0B0(0, 0xA, 0x280, DSTR(0xB459Au));            /* 0x44F97 "Scoring History" */
    f39F90(1);                                        /* 0x44FA1 */
    fg_setcolor(0xF);                                 /* 0x44FA8 */
    fg_move(0x2D, 0x190);                             /* 0x44FB7 */
    fg_draw(0x23F, 0x190);                            /* 0x44FC9 the rounds axis */
    for (n = 1; n <= 0xA; n++) {                      /* 0x44FD6; 0x44FEC cmp 0xA; jg */
        fg_setcolor(7);                               /* 0x44FF8 */
        fg_move(0x2D, 0x190 - gstep * n);             /* 0x45011 */
        fg_dash(0x23F, 0x190 - gstep * n, 0x101);     /* 0x45032 the dotted line */
        fg_move(0x2D, 0x190 - gstep * n);             /* 0x4504B */
        if (n % 5 != 0)                               /* 0x45061 idiv 5; 0x45073 test edx, edx */
            fg_draw(0x27, 0x190 - gstep * n);         /* 0x44FDB, 0x44FDE */
        else
            fg_draw(0x29, 0x190 - gstep * n);         /* 0x4507B, 0x44FDE */
    }
    fg_setcolor(0xF);                                 /* 0x45085 */
    snprintf(buf, sizeof buf, "%d", (int)ymax);       /* 0x4509B itoa */
    f3A3A0(0x23, 0x23, buf);                          /* 0x450AD */
    snprintf(buf, sizeof buf, "%d", (int)(ymax / 2)); /* 0x450B5..0x450BD (v - (v >> 31)) >> 1; 0x450CA itoa */
    f3A3A0(0x23, 0x190 - 5 * gstep - 5, buf);         /* 0x450F0 */
    fg_move(0x2D, 0x190);                             /* 0x450FC */
    fg_draw(0x2D, 0x28);                              /* 0x45108 the score axis */
    for (n = 1; n <= rounds; n++) {                   /* 0x45115; 0x45123 cmp; jg 0x451F7 */
        fg_setcolor(7);                               /* 0x4512E */
        fg_move(xstep * n + 0x2D, 0x190);             /* 0x45146 */
        fg_dash(xstep * n + 0x2D, 0x28, 0x101);       /* 0x45160 */
        fg_move(xstep * n + 0x2D, 0x190);             /* 0x45178 */
        fg_draw(xstep * n + 0x2D, 0x193);             /* 0x45190 the tick */
        if (rounds > 0x24 && (n & 1) != 0)            /* 0x4519B jle 0x451B6; 0x451A0 (again); 0x451AD test dl, 1 */
            continue;                                 /* more than 36 rounds: odd rounds unlabelled */
        snprintf(buf, sizeof buf, "%d", (int)n);      /* 0x451C4 itoa */
        fg_setcolor(0xF);                             /* 0x451CB */
        f3A0B0(xstep * n + 0x1A, 0x196, 0x28, buf);   /* 0x451ED */
    }
    if (!g_original)
        fg_setcolor(0xF);   /* BUG(1995): otherwise the colour the last round label left (7 when it was skipped) */
    f3A0B0(0x2D, 0x1A4, 0x212, DSTR(0xB45AAu));       /* 0x4520B "R O U N D S" */

    /* the legend, highest score first (0x45210..0x45323) */
    li = 0;                                           /* 0x45212 */
    for (n = 0; n < cnt; n++) {                       /* 0x45215; 0x45223 cmp; jge 0x45328 */
        nn = pair[n][1];                              /* 0x4522F dword [ebp + 4n - 0x6C] sar 16 */
        fg_setcolor(0xF);                             /* 0x4523B */
        f3A0B0(0x23F, 40 * li + 0x28, 0x3C, DSTR(D32(0xC768Cu + 4u * (uint32_t)nn)));   /* 0x45269 the name */
        fg_setcolor(S16(0xC2838u + 2u * (uint32_t)nn));   /* 0x45271 dword [2nn + 0xC2836] sar 16; 0x4527C */
        fg_move(0x249, 40 * li + 0x34);               /* 0x4529C */
        fg_dash(0x271, 40 * li + 0x34, S16(0xC284Cu + 2u * (uint32_t)nn));   /* 0x452A7 [2nn + 0xC284A] sar 16; 0x452CA */
        fg_setcolor(0xF);                             /* 0x452D4 */
        snprintf(buf, sizeof buf, "%d", (int)pair[n][0]);   /* 0x452DF dword [ebp + 4n - 0x6E] sar 16; 0x452F1 */
        f3A0B0(0x23F, 40 * li + 0x36, 0x3C, buf);     /* 0x45318 the score */
        li++;                                         /* 0x45320 */
    }

    /* the lines (0x45328..0x45405) */
    if (ymax != 0) {                                  /* 0x45328 cmp [ebp-0x30], 0; je 0x45407 */
        for (n = 1; n <= 9; n++) {                    /* 0x45332; 0x45341 cmp 9; jg */
            if (D8(nat(n, 0x9A)) == 0)                /* 0x45352 */
                continue;
            fg_setcolor(S16(0xC2838u + 2u * (uint32_t)n));   /* 0x4535F; 0x4536A */
            fg_move(0x2D, 0x190);                     /* 0x45379 */
            for (k = 1; k <= rounds; k++) {           /* 0x45386; 0x45394 cmp; jg 0x4533B */
                y = (int32_t)S16(hist(n, k)) * 360 / ymax;   /* 0x453B1 sar 16; 0x453BB..0x453CC *360; 0x453D6 idiv */
                fg_dash(xstep * k + 0x2D, 0x190 - y, S16(0xC284Cu + 2u * (uint32_t)n));   /* 0x453DE; 0x453FD */
            }
        }
    }

    if (D8(UI_SETUP_FLAGS) & 0x80) {                  /* 0x45407 test byte [0xE1624], 0x80: play-by-email */
        fg_setcolor(0xF);                             /* 0x45412 */
        snprintf(buf, sizeof buf, DSTR(0xB45B6u), (int)r);   /* 0x45435 "Round %d score may not accurately ..." */
        f3A0B0(0, 0x1B8, 0x280, buf);                 /* 0x45444 */
    }
    fg_vbclose();                                     /* 0x45449 */
    f3BB48(DLG_CONT, 0x2A, 0x109);                    /* 0x4545D CONTINUE x 265 */
    f3BC64(DLG_CONT, 0x2A, 0x1C7);                    /* 0x45471 y 455 */
    f3B05C(DLG_CONT);                                 /* 0x4547B */
    f3CC14();                                         /* 0x45480: the back buffer to the screen */
    do
        code = f78BA4(DLG_CONT, 0);                   /* 0x4548C */
    while (code != -1);                               /* 0x45497 -100; 0x4549F -1 */
    f3CC68();                                         /* 0x454A4: the background buffer (the screen from before) */
}

/* ---- 0x44A34: panel button 8, the nation's map view -------------------------------------------------------------- */

void f44A34(void)
{
    int8_t saved_pic;                                 /* [ebp-0x18] */
    uint32_t saved_dlg;                               /* [ebp-0x1C] */
    int32_t code;                                     /* [ebp-0x20] */

    saved_pic = S8(0xEE168u);                         /* 0x44A49 */
    f3CD6C(3);                                        /* 0x44A56 egypt5.gif into [0xF52CC] */
    setD8(0xEE168u, 3);                               /* 0x44A80 (before the call: mov ah, 3) */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BGBUF), S32(BACKBUF));   /* 0x44A86 */
    f78884();                                         /* 0x44A8E */
    junk_save(0, 0x27F, 0, 0x1DF, DSTR(0xB4580u));    /* 0x44AA6 fg_makegif(0, 639, 0, 479, "junk"): the screen */
    f788AC();                                         /* 0x44AAE */
    f7876C();                                         /* 0x44AB3 the phase title */
    saved_dlg = D32(UI_CUR_DIALOG);                   /* 0x44AB8 */
    f7853C(DLG_CONT);                                 /* 0x44ACF CONTINUE appended to the panel 0xF596C */
    f3BB48(PANEL_DLG, 0x2A, 0x109);                   /* 0x44AD9 (edx, ebx kept across 0x7853C) */
    f3BC64(PANEL_DLG, 0x2A, 0x1A9);                   /* 0x44AED */
    f3BFBC(PANEL_DLG, 5, 0);                          /* 0x44AFE */
    f3BFBC(PANEL_DLG, 3, 0);                          /* 0x44B0F */
    f3BFBC(PANEL_DLG, 7, 0);                          /* 0x44B20 */
    f3BFBC(PANEL_DLG, 6, 0);                          /* 0x44B31 */
    f3BFBC(PANEL_DLG, 4, 0);                          /* 0x44B42 */
    f3BFBC(PANEL_DLG, 0x101, 0);                      /* 0x44B53 */
    f783B8(PANEL_DLG, S32(0xF5940u));                 /* 0x44B63 */
    f3B05C(PANEL_DLG);                                /* 0x44B6D */
    f3F670();                                         /* 0x44B72 no border highlights */
    f459E4(S32(0xF40F8u));                            /* 0x44B7C */
    f3D9FC(S32(0xF40F8u));                            /* 0x44B86 the nation's home view */
    f3CBBC();                                         /* 0x44B8B */
    do
        code = f78BA4(PANEL_DLG, 0);                  /* 0x44B97 */
    while (code != -1);                               /* 0x44BA2 -100; 0x44BAA -1 */
    f78664(DLG_CONT);                                 /* 0x44BBE */
    f3BFBC(PANEL_DLG, 5, 1);                          /* 0x44BC8 */
    f3BFBC(PANEL_DLG, 3, 1);                          /* 0x44BDC */
    f3BFBC(PANEL_DLG, 7, 1);                          /* 0x44BF0 */
    f3BFBC(PANEL_DLG, 6, 1);                          /* 0x44C04 */
    f3BFBC(PANEL_DLG, 4, 1);                          /* 0x44C18 */
    f3BFBC(PANEL_DLG, 0x101, 1);                      /* 0x44C2C */
    setD32(UI_CUR_DIALOG, saved_dlg);                 /* 0x44C3B */
    fg_vbopen(S32(BACKBUF));                          /* 0x44C40 */
    fg_move(0, 0);                                    /* 0x44C4C */
    junk_show(DSTR(0xB4585u), 3);                     /* 0x44C5B fg_showgif("junk", 3) into the back buffer */
    fg_vbclose();                                     /* 0x44C63 */
    f78884();                                         /* 0x44C68 */
    fg_vbpaste(0, 0x27F, 0, 0x1DF, 0, 0x1DF);         /* 0x44C82 (the back buffer, still the last opened) */
    f788AC();                                         /* 0x44C8A */
    f3CD6C(saved_pic);                                /* 0x44C8F movsx; 0x44C93 */
    setD8(0xEE168u, (uint8_t)saved_pic);              /* 0x44C9B */
}

/* ---- The dispatcher's cases (0x3FB94: code = [ebp-0x3C]) --------------------------------------------------------- */

/* id 8 -> 0x3FC4B */
int dcase_8(int code)
{
    f44A34();                                         /* 0x3FC4B */
    return code;                                      /* 0x3FC50 mov eax, [ebp-0x3C] (-100) */
}

/* id 0x56 (SCORING) -> 0x3FD07 */
int dcase_56(int code)
{
    f3CC68();                                         /* 0x3FD07 */
    f44D5C();                                         /* 0x3FD11 (edi = -1 set at 0x3FD0C, kept across the call) */
    code = -1;                                        /* 0x3FD16 mov [ebp-0x3C], edi */
    return code;                                      /* 0x3FD19 */
}

/* id 0xE2 (AUDIO/VISUALS) -> 0x3FD49 */
int dcase_E2(int code)
{
    f3CC68();                                         /* 0x3FD49 */
    f3CCBC();                                         /* 0x3FD4E */
    f43E88();                                         /* 0x3FD58 (edx = -1 set at 0x3FD53, kept across the call) */
    code = -1;                                        /* 0x3FD5D mov [ebp-0x3C], edx */
    return code;                                      /* 0x3FD60 */
}
