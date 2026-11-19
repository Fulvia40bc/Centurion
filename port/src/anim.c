/* Module ANIM of docs/M6_PLAN.md:
     0x7A0A0  f7A0A0  the calamity animation and sound (the kaboom3 counter, one game_rand roll)    1,024 bytes
     0x79ADC  f79ADC  an FLC with a sound (Zeus: zeus.flc, zeus.wav)                                  404
     0x79C70  f79C70  a GIF with a sound (the BRPC picture: brpc.gif)                                 500
     0x7A4A0  f7A4A0  forget the last animation                                                        36
     0x7A4C4  f7A4C4  the current calamity's animation if animations are on                            44
   Literal translations; the address of each instruction a line comes from is in its comment. Prototypes: anim.h.
   Grades: D = read in the code at the address given, G = inference.

   Watcom C runtime -> standard C:
     0x7DB3D sprintf(buf, "%s", name) (the formats 0xB95DE, 0xB95ED, 0xB95F0, 0xB95F3, 0xB961A, all "%s") ->
             snprintf bounded to the buffer: [ebp-0x9C] of 0x7A0A0, 0x6C bytes (up to the id at [ebp-0x30])
     0x7D8C8 fopen, 0x912D5 fwrite(buf eax, size edx, n ebx, fp ecx), 0x7D95E fclose -> the same, as menu2.c's
             0x43E88: fopen("aciv.cfg", "wb") (0xB95E4 / 0xB95E1 and 0xB95F9 / 0xB95F6) relative to the current
             directory, which is the game folder in the original and in the port (main.c's plat_chdir), then
             fwrite(0xF097C, 6, 1, fp) and fclose
     0x822B5 malloc / 0x877E6 free -> load_heap_alloc / load_heap_ptr / load_heap_free (load.c's handle scheme, as
             cal5.c does for 0x1D858's local blocks and choose.c for its cat_size / malloc / cat_load / fg_imagebuf
             blocks, its closest precedent: 0x2FA34). The handle stays in a local, as the pointer did.
     0x92420 _dos_read(handle eax, count edx, buffer ecx:ebx, &bytes on the stack) on a catalog handle -> fg.h's
             cat_read(handle, buffer, count, &bytes), from where cat_size left the file pointer (the entry's data)
   __CHK dropped. The debug log 0x78B0C ("Animations" 0xB9608, 0x7A39F) is a no-op and left out (as adv.c, cal1.c).
   Library calls through fg.h in the original argument order (Fastgraph is cdecl: the pushes read right to left).

   0x8240F fg_imagebuf (as choose.c, game.h's "Pictures from memory"): ui.h's f8240F writes Fastgraph/Image's fields
   in the data object, fg.c's fg_imagebuf sets the buffer its fg_showflic(.., 4) reads; each call of 0x8240F here
   does both (imagebuf() below). The buffer is restored to ui_heap_F522C, 0xFF00 ([0xF522C], 0x79C30 / 0x7A3FA), so
   fg.c keeps no pointer to the freed FLC block. Port guard (G, as choose.c's imagebuf): a block that could not be
   allocated (a cat_size error -1 / -2 makes a size no malloc gives) gets a 1-byte buffer, so fg_showflic finds no
   FLC magic and returns 2 instead of reading through a null pointer; cat_load is skipped then (menu2.c's guard of
   0x7B74E). D: every FLC the game names is in ACIV.CT4 (anim_test), so neither guard acts in play.

   fg_makegif / fg_showgif "junk2" (port choice, as info.c, kit.c): the area under the picture is saved with
   fg_makegif(x, x + w - 1, y, y + h - 1, "junk2") (0x8CE50, not in fg.c) and put back with fg_move(x, y),
   fg_showgif("junk2", 3): junk.c's shared slot "junk2" (no file left in the game folder).

   Miles AIL calls (port no-ops, as menu2.c, marked "AIL (port no-op)"): the data reads of their arguments stay
   ([0xF6278], sample slot 0); none writes the data object. AIL_sample_status (0x843E0) returns 2 (SMP_DONE), so the
   busy wait of 0x7A43D ends at its first test; AIL_start_sample (0x84054) and AIL_end_sample (0x8419B) return nothing
   anyone reads. WAVs are loaded into sample slot 0 through menu2.c's f7B504 (which returns at once without digital
   sound).

   Data (D, read in the code at the addresses given):
     int32 [0xCAC60], [0xCAC64]: the last animation shown (0x7A0CC, 0x7A0D1; -1 after f7A4A0); only [0xCAC64] is read
     uint8 [0xF097C] aciv.cfg byte 0: 0x01 sound effects, 0x04 animations
     uint16 [0xF0980] aciv.cfg bytes 4..5: bits 9..14 the kaboom3 counter (byte [0xF0981] bits 1..6); the other bits
       are kept. 0xF097C..0xF0981 are the 6 bytes of aciv.cfg
     int32 [0x11AED8] 1 while the alternative sound plays (set at 0x7A1A6, cleared at 0x7A1F0 / 0x7A218 / 0x7A241 /
       0x7A452)
     0xCAA38[id] the calamity's WAV, 0xCAA6C[id] the alternative WAV, 0xCAAA0[id] its FLC, pointer tables indexed by
       the calamity id itself (valid 0x12..0x1E, so the first entry read is at base + 0x48; the three tables overlap
       below that). obj2: earthquk, volcano, treach, famine, superstn, civilwar, slavervt, flood, barbarns, epidemic,
       civildis, iconclsm, piracy (.wav / .flc); the alternative "kaboom3.wav" for 0x12 Earthquake and 0x13 Volcano,
       "" for the others
     int32 [0xF40E0] ACIV.CT4's catalog handle (FLCs), [0xF40D4] ACIV.CT1's (GIFs); int8 [0xF625C] digital sound on;
       int32 [0xF6278] sample slot 0's AIL handle; int32 [0xF52C8] the back buffer's handle
   The picture is centred in the map window (D: 0x79B69..0x79B98, 0x79D09..0x79D37, 0x7A30B..0x7A33A): x = (0x270 - w)
   / 2 + 8, y = (0x13F - h) / 2 + 6 with a signed halving (sar with the sign fixed: C's /). Every FLC is 220 x 150 at
   (210, 90); brpc.gif is 320 x 200 at (160, 65) (D: the catalogs, anim_test).
   Heap pointers in data fields: none (the FLC block lives in a local).

   Eggs (OVERVIEW §6, M6_PLAN "Eggs and jokes"):
     - kaboom3.wav (0x7A119..0x7A2B2): while the counter is 0, every calamity animation rolls game_rand once; a roll
       of 0 (1 in 10: r * 10 >> 15 for r = 0..3276) on Earthquake or Volcano plays kaboom3.wav instead, sets the
       counter to 51 and saves it in aciv.cfg, and the game then waits until kaboom3 has ended (0x7A43D) and loads the
       normal sound into slot 0 (not started). Each later animation counts down by one and rewrites aciv.cfg, so the
       next kaboom3 can come only 51 calamity animations later, across games and sessions (the counter lives in
       aciv.cfg). A roll of 0 on any other calamity, or a roll of 1..9, leaves the counter at 0 and aciv.cfg alone.
     - Zeus (0x79ADC, from f79270's -298): zeus.flc with zeus.wav (both identical to Iconoclasm's: OVERVIEW §6 13).
     - The BRPC (0x79C70, from f79270's -299): brpc.gif, the Barbarian Ravaging and Plundering Committee (the text at
       0xBDDB6 spells it out), shown even with animations off.

   1995 bugs: none that needed a g_original branch.
   Oddities kept (D):
     - 0x7A0A0's animations-off paths are dead: both callers test bit 0x04 of [0xF097C] first. There the FLC header,
       the size and the position are never read, so the original leaves them uninitialised; the port starts them at 0.
     - The sound of a calamity is cut at the end of its animation (AIL_end_sample, 0x7A48D) unless it is kaboom3,
       which the game waits for; Zeus's and the BRPC's sounds are not cut.
     - 0x79C70 reads the GIF's 13-byte screen descriptor, skips 768 bytes one _dos_read call each (a 256-colour
       global table) and takes w / h from the next 10 bytes, so it assumes the image descriptor follows the table
       directly. brpc.gif is a GIF87a with flags 0xF7 and its image descriptor there (D: ACIV.CT1, anim_test).
     - 0x79C70 calls f7B504(0, wav) even with an empty name (f79270 passes ""): with digital sound on that ends and
       frees whatever slot 0 held. Its two callers load brpc.wav into slot 0 only after it (cal5.c 0x1C6AF). */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "anim.h"
#include "fg.h"
#include "game.h"
#include "junk.h"
#include "menu2.h"
#include "state.h"
#include "ui.h"

/* Defined in ui_screen.c, not yet in ui.h (choose.c and title.c declare it the same way): the port's home of [0xF522C],
   the fg_imagebuf buffer (0xFF00 bytes). */
extern uint8_t *ui_heap_F522C;

#define LAST_SHOWN 0xCAC64u  /* int32 the last animation shown */
#define LAST_SET   0xCAC60u  /* int32 written with it, never read here */
#define CFG0       0xF097Cu  /* uint8 aciv.cfg byte 0: 0x01 effects, 0x04 animations; the 6-byte image starts here */
#define CFG_WORD   0xF0980u  /* uint16 aciv.cfg bytes 4..5: bits 9..14 the kaboom3 counter */
#define CFG_HI     0xF0981u  /* its high byte (counter in bits 1..6) */
#define ALT_PLAYS  0x11AED8u /* int32 1 while the alternative sound plays */
#define TAB_WAV    0xCAA38u  /* char *[id] the calamity's WAV */
#define TAB_ALT    0xCAA6Cu  /* char *[id] the alternative WAV ("kaboom3.wav" or "") */
#define TAB_FLC    0xCAAA0u  /* char *[id] the calamity's FLC */
#define CT4        0xF40E0u  /* int32 ACIV.CT4's catalog handle */
#define CT1        0xF40D4u  /* int32 ACIV.CT1's catalog handle */
#define DIGITAL    0xF625Cu  /* int8 digital sound on */
#define SAMPLE0    0xF6278u  /* int32 sample slot 0's AIL handle */
#define BACKBUF    0xF52C8u  /* int32 the back buffer's virtual-buffer handle */

/* ---- Miles AIL 3.03: port no-ops (file comment) ----------------------------------------------------------------- */

static int32_t ail_sample_status(uint32_t sample)          /* AIL (port no-op) 0x843E0 */
{
    (void)sample;
    return 2;                                         /* SMP_DONE: nothing plays */
}

static void ail_start_sample(uint32_t sample)               /* AIL (port no-op) 0x84054 */
{
    (void)sample;
}

static void ail_end_sample(uint32_t sample)                 /* AIL (port no-op) 0x8419B */
{
    (void)sample;
}

/* ---- port helper ------------------------------------------------------------------------------------------------- */

/* 0x8240F fg_imagebuf(block, size) for a local heap block: f8240F for Fastgraph/Image's fields in the data object,
   fg_imagebuf for the buffer fg.c's fg_showflic reads (file comment; the port guard as choose.c's imagebuf) */
static void imagebuf(uint8_t *p, uint32_t size)
{
    static uint8_t none[1];

    f8240F(p, size);
    if (p == NULL && size != 0)
        fg_imagebuf(none, 1);
    else
        fg_imagebuf(p, size);
}

/* ---- 0x79ADC: an FLC with a sound (Zeus) ------------------------------------------------------------------------- */

void f79ADC(const char *name, const char *wav)
{
    long size;                                        /* [ebp-0x1C] */
    unsigned nread;                                   /* [ebp-0x18] */
    uint8_t hdr[0x82] = { 0 };                        /* [ebp-0xB4] (port: zeroed, a failed read leaves the
                                                         original's stack bytes) */
    uint32_t block;                                   /* [ebp-0x14] */
    uint8_t *p;
    int w, h;                                         /* [ebp-0x28], [ebp-0x24] */
    int32_t x, y;                                     /* [ebp-0x30], [ebp-0x2C] */

    size = cat_size(name, S32(CT4));                  /* 0x79B03 cat_size(name, [0xF40E0]) */
    (void)cat_read(S32(CT4), hdr, 0x82, &nread);      /* 0x79B24 _dos_read([0xF40E0], hdr, 0x82, &nread) */
    f7B504(0, wav);                                   /* 0x79B2E */
    block = load_heap_alloc((uint32_t)size);          /* 0x79B36 malloc(size) */
    p = load_heap_ptr(block, (uint32_t)size);
    if (p != NULL)                                    /* port guard (file comment) */
        cat_load(name, p, S32(CT4));                  /* 0x79B4D cat_load(name, block, [0xF40E0]) */
    fg_flicsize(hdr, &w, &h);                         /* 0x79B64 */
    x = (0x270 - w) / 2 + 8;                          /* 0x79B69..0x79B82 */
    y = (0x13F - h) / 2 + 6;                          /* 0x79B85..0x79B9B */
    fg_move(x, y);                                    /* 0x79BA6 */
    f78884();                                         /* 0x79BAE */
    junk_makegif(x, x + w - 1, y, y + h - 1, DSTR(0xB95A1u));   /* 0x79BD0 fg_makegif(.., "junk2") (port: junk.c) */
    imagebuf(p, (uint32_t)size);                      /* 0x79BE0 fg_imagebuf(block, size) */
    if (D8(DIGITAL) != 0 && (D8(CFG0) & 0x01) != 0)   /* 0x79BEE test ah, ah; 0x79BF2 test byte [0xF097C], 1 */
        ail_start_sample(D32(SAMPLE0));               /* 0x79C02 */
    fg_showflic(DSTR(0xB95A7u), 1, 6);                /* 0x79C13 fg_showflic("", 1, 6): once, from the buffer, at the
                                                         cursor */
    f788AC();                                         /* 0x79C1B */
    fg_move(x, y);                                    /* 0x79C28 */
    f8240F(ui_heap_F522C, 0xFF00);                    /* 0x79C3C fg_imagebuf([0xF522C], 0xFF00) */
    fg_imagebuf(ui_heap_F522C, 0xFF00);               /* port: fg.c's buffer too (file comment) */
    f78884();                                         /* 0x79C44 */
    junk_showgif(DSTR(0xB95A8u), 3);                  /* 0x79C50 fg_showgif("junk2", 3) (port: junk.c) */
    f788AC();                                         /* 0x79C58 */
    load_heap_free(block);                            /* 0x79C60 free(block) */
}

/* ---- 0x79C70: a GIF with a sound (the BRPC) ---------------------------------------------------------------------- */

void f79C70(const char *gif, const char *wav)
{
    uint8_t desc[0x17];                               /* [ebp-0x48]: the screen descriptor (13), the image descriptor
                                                         (10) at +0xD; [ebp-0x2C] points at it */
    uint8_t skip[4];                                  /* [ebp-0x14]: one palette byte per read */
    unsigned nread;                                   /* [ebp-0x20] */
    int32_t i;                                        /* [ebp-0x18] */
    int32_t x, y;                                     /* [ebp-0x18] (reused), [ebp-0x28] */

    memset(desc, 0, sizeof desc);                     /* port: a short read leaves the original's stack bytes */
    (void)cat_size(gif, S32(CT1));                    /* 0x79C9B cat_size(gif, [0xF40D4]) -> [ebp-0x1C], not read */
    (void)cat_read(S32(CT1), desc, 0xD, &nread);      /* 0x79CB9 _dos_read([0xF40D4], desc, 13, &nread) */
    for (i = 0; i < 0x300; i++)                       /* 0x79CC0; 0x79CCB cmp 0x300; jge */
        (void)cat_read(S32(CT1), skip, 1, &nread);    /* 0x79CE7 the 768 palette bytes, one at a time */
    (void)cat_read(S32(CT1), desc + 0xD, 0xA, &nread);   /* 0x79D04 the image descriptor */
#define GW ((int32_t)(int16_t)(desc[0x12] | desc[0x13] << 8))   /* movsx word [ebp-0x36]: the image's width */
#define GH ((int32_t)(int16_t)(desc[0x14] | desc[0x15] << 8))   /* movsx word [ebp-0x34]: its height */
    x = (0x270 - GW) / 2 + 8;                         /* 0x79D09..0x79D20 */
    y = (0x13F - GH) / 2 + 6;                         /* 0x79D23..0x79D3A */
    fg_vbopen(S32(BACKBUF));                          /* 0x79D44 */
    fg_move(x, y);                                    /* 0x79D54 */
    f78884();                                         /* 0x79D5C */
    junk_makegif(x, GW + x - 1, y, GH + y - 1, DSTR(0xB95AEu));   /* 0x79D80 fg_makegif(.., "junk2") (port: junk.c) */
    f788AC();                                         /* 0x79D88 */
    fg_move(x, y);                                    /* 0x79D95 */
    f7B504(0, wav);                                   /* 0x79DA2 */
    cat_show(gif, 3, S32(CT1));                       /* 0x79DB4 cat_show(gif, 3, [0xF40D4]) into the back buffer */
    if (D8(DIGITAL) != 0 && wav[0] != 0 && (D8(CFG0) & 0x01) != 0)   /* 0x79DC2; 0x79DC9 movsx; 0x79DD0 */
        ail_start_sample(D32(SAMPLE0));               /* 0x79DE0 */
    fg_vbclose();                                     /* 0x79DE8 */
    f78884();                                         /* 0x79DED */
    fg_vbpaste(x, GW + x - 1, y, GH + y - 1, x, GH + y - 1);   /* 0x79E19: the area to the screen, lower-left (x, y+h-1) */
    fg_vbopen(S32(BACKBUF));                          /* 0x79E28 */
    fg_move(x, y);                                    /* 0x79E38 */
    junk_showgif(DSTR(0xB95B4u), 3);                  /* 0x79E47 fg_showgif("junk2", 3) (port: junk.c): the back
                                                         buffer as it was */
    fg_vbclose();                                     /* 0x79E4F */
    f788AC();                                         /* 0x79E54 */
#undef GW
#undef GH
}

/* ---- 0x7A0A0: the calamity animation and sound ------------------------------------------------------------------- */

void f7A0A0(int id)
{
    long size = 0;                                    /* [ebp-0x18] (port: 0, file comment) */
    unsigned nread;                                   /* [ebp-0x28] */
    uint8_t hdr[0x82] = { 0 };                        /* [ebp-0x120] (port: zeroed, file comment) */
    char wav[0x6C];                                   /* [ebp-0x9C] */
    FILE *fp;                                         /* [ebp-0x1C] */
    uint32_t block = 0;                               /* [ebp-0x20] */
    uint8_t *p = NULL;
    int w = 0, h = 0;                                 /* [ebp-0x2C], [ebp-0x34] */
    int32_t x = 0, y = 0;                             /* [ebp-0x38], [ebp-0x24] (port: 0, file comment) */
    uint32_t v;
    uint8_t b;
    uint16_t dx;

    if (S32(LAST_SHOWN) == id)                        /* 0x7A0C0 cmp eax, [ebp-0x30]; je 0x7A495 */
        return;
    setD32(LAST_SHOWN, (uint32_t)id);                 /* 0x7A0CC */
    setD32(LAST_SET, (uint32_t)id);                   /* 0x7A0D1 */
    if ((D8(CFG0) & 0x04) != 0) {                     /* 0x7A0DC test ah, 4; je 0x7A119 */
        size = cat_size(DSTR(D32(TAB_FLC + 4u * (uint32_t)id)), S32(CT4));   /* 0x7A0F3 cat_size(0xCAAA0[id], CT4) */
        (void)cat_read(S32(CT4), hdr, 0x82, &nread);  /* 0x7A114 _dos_read([0xF40E0], hdr, 0x82, &nread) */
    }

    if ((uint32_t)(uint16_t)(D16(CFG_WORD) << 1) >> 10 == 0) {   /* 0x7A119 mov ax; add eax, eax; shr ax, 0xA;
                                                                    and eax, 0xFFFF; jne 0x7A223 */
        uint32_t r = ((uint32_t)f78AE4() * 10u) >> 15;   /* 0x7A130 game_rand; 0x7A137..0x7A13E edx = r*5*2 >> 15 */
        if (r == 0) {                                 /* 0x7A147 test edx, edx; jne 0x7A1FB */
            if (S8(D32(TAB_ALT + 4u * (uint32_t)id)) != 0) {   /* 0x7A155 movsx edx, byte [0xCAA6C[id]]; je 0x7A1D3 */
                snprintf(wav, sizeof wav, "%s", DSTR(D32(TAB_ALT + 4u * (uint32_t)id)));   /* 0x7A175 (0xB95DE) */
                b = (uint8_t)(D8(CFG_HI) & 0x81);     /* 0x7A17A mov dl, [0xF0981]; and dl, 0x81 */
                setD8(CFG_HI, b);                     /* 0x7A188 */
                setD8(CFG_HI, (uint8_t)(b | 0x66));   /* 0x7A18E mov dh, dl; or dh, 0x66; 0x7A19B: the counter = 51 */
                setD32(ALT_PLAYS, 1);                 /* 0x7A1A6 */
                fp = fopen(DSTR(0xB95E4u), DSTR(0xB95E1u));   /* 0x7A1AC fopen("aciv.cfg", "wb") */
                if (fp != NULL) {                     /* 0x7A1B7 test; je 0x7A2B4 */
                    (void)fwrite(DP(CFG0), 6, 1, fp); /* 0x7A2A7 fwrite(0xF097C, 6, 1, fp) */
                    fclose(fp);                       /* 0x7A2AF */
                }
            } else {                                  /* 0x7A1D3: no alternative for this calamity */
                snprintf(wav, sizeof wav, "%s", DSTR(D32(TAB_WAV + 4u * (uint32_t)id)));   /* 0x7A1E8 (0xB95ED) */
                setD32(ALT_PLAYS, 0);                 /* 0x7A1F0 */
            }
        } else {                                      /* 0x7A1FB: r = 1..9 */
            snprintf(wav, sizeof wav, "%s", DSTR(D32(TAB_WAV + 4u * (uint32_t)id)));   /* 0x7A210 (0xB95F0) */
            setD32(ALT_PLAYS, 0);                     /* 0x7A218 */
        }
    } else {                                          /* 0x7A223: the counter runs */
        snprintf(wav, sizeof wav, "%s", DSTR(D32(TAB_WAV + 4u * (uint32_t)id)));   /* 0x7A23A (0xB95F3) */
        setD32(ALT_PLAYS, 0);                         /* 0x7A23F xor eax, eax; 0x7A241 */
        v = (uint32_t)D16(CFG_WORD) * 2u;             /* 0x7A246 mov ax, [0xF0980] (eax was 0); 0x7A24C add eax, eax */
        v = (v & 0xFFFF0000u) | ((v & 0xFFFFu) >> 10);   /* 0x7A24E shr ax, 0xA */
        b = D8(CFG_HI);                               /* 0x7A252 mov bl, [0xF0981] */
        v -= 1u;                                      /* 0x7A258 dec eax */
        b &= 0x81;                                    /* 0x7A259 and bl, 0x81 */
        v &= 0xFFFF00FFu;                             /* 0x7A25C xor ah, ah */
        setD8(CFG_HI, b);                             /* 0x7A25E: the counter's bits cleared */
        v = (v & 0xFFFFFF00u) | (v & 0x3Fu);          /* 0x7A264 and al, 0x3F */
        dx = D16(CFG_WORD);                           /* 0x7A266 mov dx, [0xF0980] (after the byte write) */
        v <<= 9;                                      /* 0x7A26D shl eax, 9 */
        dx = (uint16_t)(dx | (uint16_t)v);            /* 0x7A273 or edx, eax */
        setD16(CFG_WORD, dx);                         /* 0x7A27A: the counter - 1 */
        fp = fopen(DSTR(0xB95F9u), DSTR(0xB95F6u));   /* 0x7A286 fopen("aciv.cfg", "wb") */
        if (fp != NULL) {                             /* 0x7A291 test; je 0x7A2B4 */
            (void)fwrite(DP(CFG0), 6, 1, fp);         /* 0x7A2A7 fwrite(0xF097C, 6, 1, fp) */
            fclose(fp);                               /* 0x7A2AF */
        }
    }

    f7B504(0, wav);                                   /* 0x7A2BC */
    if ((D8(CFG0) & 0x04) != 0) {                     /* 0x7A2C1 test byte [0xF097C], 4; je 0x7A39A */
        block = load_heap_alloc((uint32_t)size);      /* 0x7A2D1 malloc(size) */
        p = load_heap_ptr(block, (uint32_t)size);
        if (p != NULL)                                /* port guard (file comment) */
            cat_load(DSTR(D32(TAB_FLC + 4u * (uint32_t)id)), p, S32(CT4));   /* 0x7A2EF cat_load(0xCAAA0[id], block, CT4) */
        fg_flicsize(hdr, &w, &h);                     /* 0x7A306 */
        x = (0x270 - w) / 2 + 8;                      /* 0x7A30B..0x7A324 */
        y = (0x13F - h) / 2 + 6;                      /* 0x7A327..0x7A33D */
        fg_move(x, y);                                /* 0x7A348 */
        f78884();                                     /* 0x7A350 */
        junk_makegif(x, x + w - 1, y, y + h - 1, DSTR(0xB9602u));   /* 0x7A372 fg_makegif(.., "junk2") (port: junk.c) */
        fg_move(x, y);                                /* 0x7A382 */
        imagebuf(p, (uint32_t)size);                  /* 0x7A392 fg_imagebuf(block, size) */
    }
    /* 0x7A39F: the debug log 0x78B0C("Animations"), a no-op */
    if (D8(DIGITAL) != 0 && wav[0] != 0 && (D8(CFG0) & 0x01) != 0)   /* 0x7A3A4; 0x7A3AD movsx; 0x7A3B8 */
        ail_start_sample(D32(SAMPLE0));               /* 0x7A3C8 */
    if ((D8(CFG0) & 0x04) != 0) {                     /* 0x7A3D0 test byte [0xF097C], 4; je 0x7A42A */
        fg_showflic(DSTR(0xB9613u), 1, 6);            /* 0x7A3E2 fg_showflic("", 1, 6): once, from the buffer, at
                                                         the cursor */
        fg_move(x, y);                                /* 0x7A3F2 */
        f8240F(ui_heap_F522C, 0xFF00);                /* 0x7A406 fg_imagebuf([0xF522C], 0xFF00) */
        fg_imagebuf(ui_heap_F522C, 0xFF00);           /* port: fg.c's buffer too (file comment) */
        junk_showgif(DSTR(0xB9614u), 3);              /* 0x7A415 fg_showgif("junk2", 3) (port: junk.c) */
        f788AC();                                     /* 0x7A41D */
        load_heap_free(block);                        /* 0x7A425 free(block) */
    }
    if (D8(DIGITAL) != 0) {                           /* 0x7A42A cmp byte [0xF625C], 0; je 0x7A495 */
        if (S32(ALT_PLAYS) != 0) {                    /* 0x7A433 test edi, edi; je 0x7A486 */
            while (ail_sample_status(D32(SAMPLE0)) != 2)   /* 0x7A443; 0x7A44B cmp eax, 2; jne 0x7A43D */
                ;
            setD32(ALT_PLAYS, 0);                     /* 0x7A452 */
            snprintf(wav, sizeof wav, "%s", DSTR(D32(TAB_WAV + 4u * (uint32_t)id)));   /* 0x7A46F (0xB961A) */
            f7B504(0, wav);                           /* 0x7A47F: the normal sound into slot 0 (not started) */
        } else
            ail_end_sample(D32(SAMPLE0));             /* 0x7A48D */
    }
}

/* ---- 0x7A4A0 ----------------------------------------------------------------------------------------------------- */

void f7A4A0(void)
{
    setD32(0xCAC60u, 0xFFFFFFFFu);                    /* 0x7A4B3 */
    setD32(0xCAC64u, 0xFFFFFFFFu);                    /* 0x7A4B9 */
}

/* ---- 0x7A4C4 ----------------------------------------------------------------------------------------------------- */

void f7A4C4(int anim)
{
    (void)anim;                                       /* 0x7A4D4 [ebp-4] = eax, never read */
    if ((D8(0xF097Cu) & 0x04) != 0)                   /* 0x7A4D7 test byte [0xF097C], 4; je */
        f7A0A0(S32(0xF783Cu));                        /* 0x7A4E5 */
}
