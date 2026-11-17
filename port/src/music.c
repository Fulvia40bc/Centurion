/* The music: the AdLib driver ADLIB.LZW (CENT.DAT; "Music Programming / Rob Hubbard" in the credits) ported literally,
 * and the game's calls into it (EXE segment 0823). Claude, 2026-11-17. docs/MUSIC_NOTES.md (the decode, section 10 the
 * port's design), disasm/ADLIB_DRV.asm (the driver, annotated), tools/adlib_regs.py (the reference model); the chip is
 * opl2.c.
 *
 *   the driver (offsets in its own segment; 5EB5 loads it at seg:0000)
 *     0x403 start (AH=1)  0x470 stop (AH=2)  0x47B enable (AH=4)  0x483 disable (AH=3)  0x4A9 tick (AH=0)
 *     0x5A6 program change  0x5CC end of the song  0x5E2 note on  0x6AB note off  0x708 drum on  0x73F voice
 *     0x7A3 pitch bend  0x7FB patch  0x929 chip reset
 *   the game  5D73 load(n)  5D29 play(n)  5D20 stop  5E1A unload(n)  072E / 074B the driver calls AH=1 / AH=2
 *             5EB5 the file  5E86 the sound's start-up  the ISR 0001's music part (AH=3/4 on [3EE9]'s edges, AH=0)
 *             03FF Ctrl+S ([3EE9] ^= FF)
 *
 * The driver is code and data in one segment: ADLIB.LZW (2426 bytes) is loaded into g_drv (64 KB, the segment: every
 * read the code can make stays inside it, as in the original) and 0x3E8..0x979 is translated routine by routine on that
 * array, its variables at their offsets and its tables (frequencies, patches, drums, register lists) the file's own. The
 * song is ES = g_es (64 KB): the start copies the slot's song into it (the slot may then be freed by 5E1A while the
 * driver plays; nothing writes a song, so this reads what the original reads).
 *
 * Time and threads: the ISR runs at the PIT's 72.83 Hz (counter 0x3FFF); one tick is 682.625 samples of the OPL2's
 * 49715.9 Hz, so music_fill keeps an accumulator (+8 per sample, a tick at 5461) and runs the ISR's music part before
 * the sample, in the SDL audio callback (sdl2_platform.c), which holds the device's lock. The game thread's driver calls
 * (072E, 074B) and its writes of [3EE9] take the same lock (platform_audio_lock): the driver's cli / sti. The audio
 * thread reads [3EEC] and [3EE9] and writes no DS byte: DS:008B, the ISR's copy of [3EE9], is g_8B here.
 *
 * When the music runs: in a window game (main.c play_game) whose audio device opens (music_init_5E86), as with an
 * AdLib found by 055C and no word on the command line ([3EEA] = 12h). Headless runs (scripts, test hooks, --music-wav)
 * never open a device and the driver calls do nothing; the DS parts do exactly what they did before (the slot table,
 * [3EEE]; [3EEC] as the DS holds it: 0 in a new game, FFFF in a DOSBox snapshot), so their results are unchanged.
 *
 * 1990 BUGS FIXED (MUSIC_NOTES 8; --original keeps them):
 *   1. The frequency table has no range check: F(i) = word [((i << 1) & FFh) + 14h], so i = note + transpose outside
 *      0..95 reads the bytes after the table (the reset's register list) as a frequency. i = 96 is fnum 0, silent: the
 *      top notes of CN9B / CN9C / CN9D (the rowing music). i = -5 / -3 / -2 are 9392h / A1A0h / A3A2h: CN4C's deep drum
 *      of the excited crowd is a high tick at block 4 fnum 392h (also CN8A, CN1). Fixed: i -12..-1 = the word of i + 12
 *      with its fnum halved (block 0); i 96..102 = the word of i - 12 with its fnum doubled (block 7). Other i as in 1990.
 *   2. The velocity's volume underflows: 73F writes carrier 40 = [39B] - vel / 4 as one byte, KSL bits included, so a
 *      loud note on a patch with a low TL borrows into KSL and comes out at TL 62-63, silent (CN8B's accents, the
 *      civilized battle). Fixed: KSL kept, TL = max(TL - vel / 4, 0).
 * Latent quirks kept (no song reaches them): the drum note-off adds the transpose; Dc skips 2 data bytes; running
 * status; a bend's overflow into the block bits; programs above 23. Not bugs (kept): CC7 ignored; drum notes other than
 * 36 / 38 / 42 / 54 not played; MIDI channels without an OPL channel skipped (the other devices' parts); a 4th note on
 * a 3-channel part dropped when all three started in the same tick; a muted (Ctrl+S) song runs on.
 *
 * EASTER EGGS / HIDDEN TEXT (MUSIC_NOTES 11; nothing new here): ROLAND.LZW 0x6E holds the MT-32 display message
 * "*ELECTRONIC ARTS*" that no code sends (that driver is not emulated). In ADLIB.LZW the random routine 0x90B (seed
 * 563Ah), 195 bytes of patches no pointer reaches, patch 22 and the jump-table entries +3..+F are unused: they stay in
 * g_drv as data.
 *
 * PORT: only the AdLib (the speaker, MT-32 and Tandy drivers are not emulated; the word "nosnd" is taken, main.c); the
 * EXE's AdLib probe 055C, the PIT set-up 0062 / 00AB and the I/O delays 0x903 / 0x921 have nothing to do; the slot's
 * handle is 1 (not a DOS segment) while a song is loaded; 5EB5's format-bit and size checks are not made (every CN*.LZW
 * passes them). The exit's 0EE7 ([3EE9] = 0, 5D20: silence the card before DOS) is not needed: the device is closed.
 * A slot a DOSBox snapshot says is loaded (a window game from --load .bin) is read when it is started (072E). Known
 * limit of that dev path (D, not run; left as it is): such a game starts silent, as the snapshot's [3EEE] (01 in every
 * t*_in.bin) names the song DOSBox was playing, so map_build's 5D29(1) does nothing until the song changes; and g_8B
 * starts at 0 (the snapshot's [8B] is FF), so the first tick sends AH=4, as after every start-up (5E86 sets [3EE9] = FF).
 * A .CSV load or a new game has [3EEE] = FF (static) and plays at once.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compat.h"
#include "music.h"
#include "opl2.h"
#include "state.h"
#include "assets.h"
#include "platform.h"

static uint8_t g_drv[0x10000];            /* the driver's segment: ADLIB.LZW at 0 */
static uint8_t g_es[0x10000];             /* ES of the tick: the song */
static uint8_t *g_slot[4];                /* the songs of the slots DS:A6B0 (5EB5's DOS blocks) */
static size_t g_slot_n[4];
static int g_on;                          /* the driver runs: an audio device is open */
static uint8_t g_8B;                      /* DS:008B, the ISR's last [3EE9] */
static unsigned g_acc;                    /* the PIT in OPL2 samples x 8 */
static FILE *g_log;                       /* --music-wav .txt: the register writes */
static unsigned g_tick, g_loops;          /* --music-wav: the tick being logged; FC 80 jumps taken */

#define DB(a) g_drv[(uint16_t)(a)]
#define ES(a) g_es[(uint16_t)(a)]
static uint16_t DW(uint16_t a) { return (uint16_t)(DB(a) | DB(a + 1) << 8); }
static void setDW(uint16_t a, uint16_t v) { DB(a) = (uint8_t)v; DB(a + 1) = (uint8_t)(v >> 8); }

/* out to 388h (the register) and 389h (the value) */
static void out(uint8_t reg, uint8_t val) {
    opl2_write(reg, val);
    if (g_log) fprintf(g_log, "%u %02X %02X\n", g_tick, reg, val);
}

/* ================================================================ the driver */

/* 0x929 chip reset: 01 = 20h (waveform select on), 04 = E0h, the 101 registers of 0x0D6 = 0 (from the last), the 18 TL
   registers of 0x13B = 3Fh (from the last) */
static void reset_929(void) {
    out(0x01, 0x20);
    out(0x04, 0xE0);
    for (int si = 0x64; si >= 0; si--) out(DB(0xD6 + si), 0);         /* 949 */
    for (int si = 0x11; si >= 0; si--) out(DB(0x13B + si), 0x3F);     /* 961 */
}

/* F(i): the frequency word [((i << 1) & FFh) + 14h] (shl bl: 8 bits), i = note + transpose; bug 1 fixed */
static uint16_t freq(uint8_t i) {
    int s = (int8_t)i;
    if (!g_original_bugs && s >= -12 && s < 0)
        return (uint16_t)((DW((uint16_t)(0x14 + 2 * (s + 12))) & 0x3FF) >> 1);
    if (!g_original_bugs && s >= 96 && s <= 102) {
        uint16_t w = DW((uint16_t)(0x14 + 2 * (s - 12)));
        return (uint16_t)((w & 0xFC00) | (w & 0x3FF) << 1);
    }
    return DW((uint16_t)(0x14 + ((i << 1) & 0xFF)));
}

/* 0x73F voice(cx = channel, bx = frequency word, ah = velocity), [3D2] = the key-on bit (20h or 0) */
static void voice_73F(int cx, uint16_t bx, uint8_t ah) {
    if (DB(0x3DD) != 1) DB(0x3D2) = 0;                                  /* 73f: disabled: never key on */
    if (DB(0x3D2) != 0) {                                                /* 755: carrier 40 = [39B] - vel / 4 */
        uint8_t b = DB(0x39B + cx), v = (uint8_t)(ah >> 2);
        uint8_t al = (uint8_t)(b - v);                                   /* 76b: sub al, ah (bug 2) */
        if (!g_original_bugs) al = (uint8_t)((b & 0xC0) | ((b & 0x3F) > v ? (b & 0x3F) - v : 0));
        out((uint8_t)(DB(0x353 + cx) + 0x40), al);
    }
    out((uint8_t)(0xA0 + cx), (uint8_t)bx);                              /* 771: A0, shadow 3A4 */
    DB(0x3A4 + cx) = (uint8_t)bx;
    uint8_t al = (uint8_t)(bx >> 8 | DB(0x3D2));                         /* 787: B0 | key-on, shadow 3AD */
    out((uint8_t)(0xB0 + cx), al);
    DB(0x3AD + cx) = al;
}

/* 0x7A3 bend(di = channel): fnum = fnum x [3D5] >> 13, block and key-on kept ([3D7]); no song bends */
static void bend_7A3(int di) {
    uint8_t ah = DB(0x3AD + di);
    DB(0x3D7) = ah & 0xFC;                                               /* 7ac */
    uint32_t p = (uint32_t)((ah & 3) << 8 | DB(0x3A4 + di)) * DW(0x3D5);  /* 7bc: mul dx */
    uint16_t bx = (uint16_t)(p >> 13);                                   /* 7be..7cc: DL:AX = p >> 8, 5 x shr / rcr */
    out((uint8_t)(0xA0 + di), (uint8_t)bx);
    out((uint8_t)(0xB0 + di), (uint8_t)(bx >> 8 | DB(0x3D7)));
}

/* 0x7FB patch(bl = program, cx = channel): record [167 + 2 x [14F + bl]] (shl bl: 8 bits) to the two operators */
static void patch_7FB(uint8_t bl, int cx) {
    uint16_t di = DW((uint16_t)(0x167 + ((DB(0x14F + bl) << 1) & 0xFF)));
    uint8_t mo = DB(0x34A + cx), ca = DB(0x353 + cx);
    out((uint8_t)(mo + 0x20), DB(di));                                   /* 80e: the modulator */
    out((uint8_t)(mo + 0x40), DB(di + 1));
    out((uint8_t)(mo + 0x60), DB(di + 2));
    out((uint8_t)(mo + 0x80), DB(di + 3));
    out((uint8_t)(ca + 0x20), DB(di + 4));                               /* 861: the carrier */
    DB(0x39B + cx) = DB(di + 5);                                         /* 887: the velocity's base */
    out((uint8_t)(ca + 0x40), DB(di + 5));
    out((uint8_t)(ca + 0x60), DB(di + 6));
    out((uint8_t)(ca + 0x80), DB(di + 7));
    out((uint8_t)(0xC0 + cx), DB(di + 8));                               /* 8b9: feedback / connection */
    DB(0x377 + cx) = DB(di + 9);                                         /* 8cc: the transpose */
    out((uint8_t)(mo + 0xE0), DB(di + 10));                              /* 8d8: waveforms */
    out((uint8_t)(ca + 0xE0), DB(di + 11));
}

/* AH=1 0x403 start(cx = the song's segment): the port copies the song into ES */
static void start_403(uint16_t cx, const uint8_t *song, size_t n) {
    setDW(0x12, cx);                                                     /* 404 */
    memset(g_es, 0, sizeof g_es);
    memcpy(g_es, song, n < sizeof g_es ? n : sizeof g_es);
    uint16_t ax = (uint16_t)(ES(0) | ES(1) << 8);                        /* 40b: the first event = the loop start */
    setDW(0x3C8, ax);
    setDW(0x3CA, ax);
    for (int di = 0; di < 9; di++) DB(0x35C + di) = ES(0x22 + di);      /* 415: the channel map */
    reset_929();                                                         /* 426 */
    for (int di = 0; di < 9; di++) {                                     /* 429: free, no note, age 0 */
        DB(0x365 + di) = 0;
        DB(0x36E + di) = 0;
        setDW((uint16_t)(0x3B6 + 2 * di), 0);
    }
    for (int di = 0; di < 9; di++)                                       /* 445: the drum channels' patches */
        if (DB(0x35C + di) == 9) patch_7FB(DB(0x392 + di), di);
    DB(0x3CE) = 0;                                                       /* 45f */
    DB(0x3D8) = 0;
    DB(0x3D1) = 1;
}
static void stop_470(void) { DB(0x3D1) = 0; reset_929(); }              /* AH=2 */
static void enable_47B(void) { DB(0x3DD) = 1; }                          /* AH=4 */
static void disable_483(void) {                                          /* AH=3: keys off (B0 = 0; shadows kept) */
    DB(0x3DD) = 0;
    for (int cl = 0; cl < 9; cl++) out((uint8_t)(0xB0 + cl), 0);
}

/* 0x5A6 C0: patch [si] on every channel mapped to this MIDI channel, not for channel 9 */
static uint16_t program_5A6(uint16_t si) {
    uint8_t bl = ES(si);
    si++;
    for (int di = 0; di < 9; di++) {
        uint8_t al = DB(0x3CD);
        if (al == 9 || al != DB(0x35C + di)) continue;                  /* 5b1 / 5b5 */
        patch_7FB(bl, di);
    }
    return si;
}

/* 0x6AB 80 / 90 with velocity 0: the first channel of this MIDI channel holding the note keys off at its frequency */
static uint16_t note_off_6AB(uint16_t si) {
    uint8_t bl = ES(si);
    si = (uint16_t)(si + 2);                                             /* 6b2: the velocity skipped */
    uint8_t al = DB(0x3CD);
    for (int di = 0; di < 9; di++) {
        if (al != DB(0x35C + di) || DB(0x36E + di) != bl) continue;      /* 6b7 / 6bd */
        DB(0x3D2) = 0;                                                   /* 6c6 */
        DB(0x365 + di) = 0;
        uint8_t n = al == 9 ? DB(0x389 + di) : DB(0x36E + di);          /* 6d0: drums their index (+ transpose: latent) */
        DB(0x36E + di) = 0;
        setDW((uint16_t)(0x3B6 + 2 * di), 0);                            /* 6f0 */
        voice_73F(di, freq((uint8_t)(n + DB(0x377 + di))), 0);          /* no key-on: AH unused */
        break;
    }
    return si;
}

/* 0x5E2 90: note on (velocity 0 = note off) */
static uint16_t note_on_5E2(uint16_t si) {
    if (ES(si + 1) == 0) return note_off_6AB(si);                       /* 5e2 */
    uint8_t bl = ES(si), ah = ES(si + 1);                                /* 5f2: note, velocity */
    si = (uint16_t)(si + 2);
    uint8_t al = DB(0x3CD);
    if (al == 9) {                                                       /* 5fd -> 708: the drum channel of this note */
        for (int di = 0; di < 9; di++) {
            if (al != DB(0x35C + di) || bl != DB(0x380 + di)) continue;
            DB(0x3D2) = 0x20;                                            /* 716: (re)keyed, busy or not */
            DB(0x365 + di) = 1;
            DB(0x36E + di) = bl;
            voice_73F(di, freq(DB(0x389 + di)), ah);                     /* 724: no transpose */
            break;
        }
        return si;
    }
    for (int di = 0; di < 9; di++) {                                     /* 604: the first free channel of c */
        if (al != DB(0x35C + di) || DB(0x365 + di) == 1) continue;
        DB(0x3D2) = 0x20;
        DB(0x365 + di) = 1;
        DB(0x36E + di) = bl;
        voice_73F(di, freq((uint8_t)(bl + DB(0x377 + di))), ah);
        return si;
    }
    uint16_t dx = 0;                                                     /* 63b: none free: the oldest (age > 0) */
    setDW(0x3D3, 0xFFFF);
    for (int di = 0; di < 9; di++) {
        if (al != DB(0x35C + di)) continue;
        uint16_t cx = DW((uint16_t)(0x3B6 + 2 * di));
        if (dx < cx) { dx = cx; setDW(0x3D3, (uint16_t)di); }            /* 654: jae = ties keep the first */
    }
    if (DW(0x3D3) == 0xFFFF) return si;                                  /* 664: all started this tick: dropped */
    int di = DW(0x3D3);
    DB(0x3D2) = 0;                                                       /* 675: key off (A0 = B0 = 0) */
    voice_73F(di, 0, ah);
    setDW((uint16_t)(0x3B6 + 2 * di), 0);
    DB(0x3D2) = 0x20;                                                    /* 68b: and on with the new note */
    DB(0x365 + di) = 1;
    DB(0x36E + di) = bl;
    voice_73F(di, freq((uint8_t)(bl + DB(0x377 + di))), ah);
    return si;
}

/* AH=0 0x4A9 tick */
static void tick_4A9(void) {
    if (DB(0x3D1) != 1) return;                                          /* 4a9: not playing */
    uint16_t si = DW(0x3C8);                                             /* 4b3: es = [12] (g_es) */
    for (int di = 0; di < 9; di++)                                       /* 4bb: the sounding channels age */
        if (DB(0x365 + di) == 1) setDW((uint16_t)(0x3B6 + 2 * di), (uint16_t)(DW((uint16_t)(0x3B6 + 2 * di)) + 1));
    if (DB(0x3CE) != 0) {                                                /* 4d2: still waiting */
        DB(0x3CE)--;
        setDW(0x3C8, si);
        return;
    }
    for (;;) {
        uint8_t al = ES(si++), ah;                                       /* 4e0: the next event */
        if (!(al & 0x80)) {                                              /* 4e4: a data byte: running status */
            ah = DB(0x3CC);
            si--;
            if (DB(0x3CF) != 0) {                                        /* 4ed: the last status was skipped */
                si++;
                if (DB(0x3D0) != 0) si++;
                goto delta;
            }
        } else if (al == 0xFC) {                                         /* 500 -> 5cc: the end */
            al = ES(si++);
            if (al != 0x80) { DB(0x3D1) = 0; return; }                   /* 5d4: stop (5a5: ret, [3C8] not stored) */
            si = DW(0x3CA);                                              /* 5db: loop */
            g_loops++;
            continue;
        } else {                                                         /* 507: channel 3CD, type 3CC */
            DB(0x3CD) = al & 0x0F;
            ah = al & 0xF0;
            DB(0x3CC) = ah;
        }
        if (ah == 0xE0) {                                                /* 519: pitch bend */
            DB(0x3D0) = 1;
            uint8_t dl = ES(si), dh = ES(si + 1);
            si = (uint16_t)(si + 2);
            if (dh & 1) dl |= 0x80;                                      /* 52b: shr dh; jae; or dl, 80h */
            dh >>= 1;
            setDW(0x3D5, (uint16_t)(dl | dh << 8));
            for (int di = 0; di < 9; di++)                               /* 539 */
                if (DB(0x3CD) == DB(0x35C + di) && DB(0x365 + di) == 1) bend_7A3(di);
            goto delta;
        }
        DB(0x3CF) = 0;                                                   /* 558 */
        DB(0x3D0) = 1;
        if (ah == 0xC0) { DB(0x3D0) = 0; si = program_5A6(si); }         /* 562 */
        else if (ah == 0x90) si = note_on_5E2(si);                       /* 56f */
        else if (ah == 0x80) si = note_off_6AB(si);                      /* 577 */
        else { si = (uint16_t)(si + 2); DB(0x3D0) = 1; DB(0x3CF) = 1; }  /* 57f: anything else, 2 data bytes */
    delta:
        al = ES(si++);                                                   /* 58e: 0 = the same tick */
        if (al != 0) { DB(0x3CE) = (uint8_t)(al - 1); break; }
    }
    setDW(0x3C8, si);                                                    /* 5a1 */
}

/* the ISR 0823:0001's music part (the clock DS:42 and the BIOS chain are the game's, naval.c nv_tick) */
static void isr_0001(void) {
    if (B(0x3EEC) == 0) return;                                          /* the sound is off (byte test) */
    if (B(0x3EE9) != g_8B) {                                             /* Ctrl+S / the start-up: AH=4 or AH=3 */
        g_8B = B(0x3EE9);
        if (g_8B) enable_47B(); else disable_483();
    }
    tick_4A9();                                                          /* AH=0 */
}

/* the SDL audio callback (the device's lock held): a PIT tick every 682.625 samples, before the sample */
static void music_fill(int16_t *buf, int n) {
    for (int i = 0; i < n; i++) {
        if ((g_acc += 8) >= 5461) { g_acc -= 5461; isr_0001(); }
        buf[i] = opl2_sample();
    }
}

/* ================================================================ the game (EXE) */

/* 5EB5's file: name[m] + ".lzw" (DS:3EF0 {name, archive}, DS:3F44) from archive 0F7B(n); 17..20 are the drivers */
static uint8_t *song_file(uint8_t m, size_t *n) {
    static const char *const arch[10] = { "MRACE", NULL, "BATL", "CENT", "MINIT", "MMAP", "MGLAD", "MNEGO", "MNAVAL", "NAVL" };
    uint16_t a = W(0x3EF2 + 4 * m);
    char name[40], dir[16], dat[16];
    if (a > 9 || !arch[a]) return NULL;
    snprintf(name, sizeof name, "%s%s", (const char *)&g_ds[W(0x3EF0 + 4 * m)], (const char *)&g_ds[0x3F44]);
    snprintf(dir, sizeof dir, "%s.DIR", arch[a]);
    snprintf(dat, sizeof dat, "%s.DAT", arch[a]);
    return assets_member(dir, dat, name, n);
}

/* 0x5D73 music_load(m) (D): the table DS:A6B0 has 4 slots {u16 handle, u8 loaded, u8 id}. Nothing when a loaded slot
   (loaded != 0) has id m (0x5D7D); else the first slot with loaded == 0 (0x5DB1; none: nothing) gets handle = 5EB5(m)
   (0x5DEE), and only when that is nonzero also id = m (0x5E02) and loaded = 0xFF (0x5E0F). 5EB5 returns 0 at once when
   the sound is off ([3EEC] == 0, word compare) and m < 0x11 (unsigned), else loads name[m] + ".lzw" and returns its DOS
   segment. The port's handle is 1 for a loaded song; with the driver running the file is read into the slot's buffer
   (0 if that fails); without it nothing is read and the load counts as done (as before the music was ported: [3EEC]
   is 0 in a new game, FFFF in a DOSBox snapshot, and then the slot is taken as in the original). */
void music_load_5D73(uint8_t m) {
    for (int i = 0; i < 4; i++)
        if (B(0xA6B2 + 4 * i) != 0 && B(0xA6B3 + 4 * i) == m) return;
    int k = 0;
    while (k < 4 && B(0xA6B2 + 4 * k) != 0) k++;
    if (k == 4) return;
    uint16_t h = (W(0x3EEC) != 0 || m >= 0x11) ? 1 : 0;
    if (h && g_on) {
        free(g_slot[k]);
        g_slot[k] = song_file(m, &g_slot_n[k]);
        h = g_slot[k] != NULL;
    }
    setW(0xA6B0 + 4 * k, h);
    if (h == 0) return;
    B(0xA6B3 + 4 * k) = m;
    B(0xA6B2 + 4 * k) = 0xFF;
}

/* 0x072E(seg) for slot i: if ([3EEC] && seg) the driver's AH=1 with CX = seg */
static void start_072E(int i) {
    if (!g_on || !W(0x3EEC) || !W(0xA6B0 + 4 * i)) return;
    if (!g_slot[i]) g_slot[i] = song_file(B(0xA6B3 + 4 * i), &g_slot_n[i]);   /* a slot of a DOSBox snapshot */
    if (!g_slot[i]) return;
    platform_audio_lock(1);
    start_403(W(0xA6B0 + 4 * i), g_slot[i], g_slot_n[i]);
    platform_audio_lock(0);
}

/* 0x5D29 music_play(m): 5D73(m) first; then, when [3EEE] != m, every slot i 0..3 whose id DS:A6B3[4i] is m (the loaded
   byte is not tested) is started (072E) and [3EEE] = m. So playing the current song again does nothing, and the game
   does not know when a song that stops (FC 81) has ended. */
void music_play_5D29(uint8_t m) {
    music_load_5D73(m);
    if (B(0x3EEE) == m) return;
    for (int i = 0; i < 4; i++)
        if (B(0xA6B3 + 4 * i) == m) { start_072E(i); B(0x3EEE) = m; }
}

/* 0x5D20 music_stop: [3EEE] = FFh, then 074B: if ([3EEC]) the driver's AH=2 (silent at once) */
void music_stop_5D20(void) {
    B(0x3EEE) = 0xFF;
    if (!g_on || !W(0x3EEC)) return;
    platform_audio_lock(1);
    stop_470();
    platform_audio_lock(0);
}

/* 0x5E1A music_unload(n): the first slot with loaded != 0 and id n (0x5E29) is freed (868C) and set to {0, 0, 0xFF},
   and [3EEE] = 0xFF (0x5E6B..0x5E7C). The driver is not stopped: every caller stops it first. */
void music_unload_5E1A(uint8_t n) {
    for (int i = 0; i < 4; i++) {
        if (B(0xA6B2 + 4 * i) == 0 || B(0xA6B3 + 4 * i) != n) continue;
        free(g_slot[i]);
        g_slot[i] = NULL;
        setW((uint16_t)(0xA6B0 + 4 * i), 0); B(0xA6B2 + 4 * i) = 0; B(0xA6B3 + 4 * i) = 0xFF; B(0x3EEE) = 0xFF;
        break;
    }
}

/* B0E0 with an AdLib found (055C) and no word: [3EEA] = 12h; then 5E86: [3EE9] = 0, [C4] = 5EB5(12h) (ADLIB.LZW at
   seg:0000; the captures' 3682:0000), [3EE9] = FF, [3EEC] = FFFF. Here: the driver into g_drv, the chip reset, the audio
   device opened; the DS part only once it runs (else the DS stays as it is: the sound off, as with no driver). */
int music_init_5E86(void) {
    size_t n = 0;
    uint8_t *d = song_file(0x12, &n);
    if (!d) { fprintf(stderr, "music: ADLIB.LZW not found in CENT.DAT\n"); return -1; }
    memcpy(g_drv, d, n < sizeof g_drv ? n : sizeof g_drv);
    free(d);
    opl2_reset();
    if (platform_audio_open(music_fill) != 0) return -1;
    platform_audio_lock(1);
    B(0x3EEA) = 0x12;
    B(0x3EE9) = 0xFF;
    setW(0xC4, 0); setW(0xC6, 0x3682);
    setW(0x3EEC, 0xFFFF);
    g_on = 1;
    platform_audio_lock(0);
    return 0;
}
void music_nosnd_B0E0(void) { B(0x3EEA) = 0xFF; B(0x3EE9) = 0; }      /* the word "nosnd", then 5E86 returns at once */
int music_on(void) { return g_on; }
/* INT 9 0x03FF: make code 1Fh (S) with Ctrl held: [3EE9] ^= FFh; the ISR sends AH=3 (keys off) or AH=4 */
void music_ctrl_s_03FF(void) {
    if (g_on) platform_audio_lock(1);
    B(0x3EE9) ^= 0xFF;
    if (g_on) platform_audio_lock(0);
}

/* ================================================================ --music-wav SONG OUT SECONDS (MUSIC_NOTES 10) */
static void le(FILE *f, uint32_t v, int n) { for (int i = 0; i < n; i++) fputc((int)(v >> (8 * i)) & 0xFF, f); }

/* SONG = 0..16, a name (cn9a) or the path of an unpacked song file. OUT *.txt: the register log "tick reg val" (hex) for
   floor(SECONDS x 1193182 / 16383) ticks, the start's writes as tick 0 (= tools/adlib_regs.py SONG SECONDS with
   --original); else a WAV, 49716 Hz mono 16-bit, SECONDS x 49715.9 samples (0 < SECONDS <= 43200: the RIFF sizes).
   Run in the game folder; no SDL. A song file is played as the driver finds it: one with no event to stop on (e.g. all
   zeros: loop start 0, every delta 0) never ends its first tick, here as in 1990 (seen in review; not checked for). */
int music_wav(const char *song, const char *outp, double seconds) {
    if (!(seconds > 0 && seconds <= 43200)) { fprintf(stderr, "SECONDS must be > 0 and at most 43200 (12 hours)\n"); return 1; }
    if (state_init("OVL0.OVL") != 0) { fprintf(stderr, "cannot read OVL0.OVL (run in the game folder)\n"); return 1; }
    size_t n = 0;
    uint8_t *s = song_file(0x12, &n);
    if (!s) { fprintf(stderr, "ADLIB.LZW not found in CENT.DAT\n"); return 1; }
    memcpy(g_drv, s, n < sizeof g_drv ? n : sizeof g_drv);
    free(s);
    s = NULL;
    char *end;
    long m = strtol(song, &end, 10);
    if (*song && !*end && m >= 0 && m < 17) s = song_file((uint8_t)m, &n);
    else {
        for (m = 0; m < 17 && strcasecmp(song, (const char *)&g_ds[W(0x3EF0 + 4 * m)]) != 0; m++) {}
        if (m < 17) s = song_file((uint8_t)m, &n);
        else {
            FILE *f = fopen(song, "rb");
            if (f) {
                s = malloc(0x10000);
                n = s ? fread(s, 1, 0x10000, f) : 0;
                fclose(f);
            }
        }
    }
    if (!s) { fprintf(stderr, "song %s not found\n", song); return 1; }
    size_t l = strlen(outp);
    int txt = l > 4 && strcasecmp(outp + l - 4, ".txt") == 0;
    FILE *o = fopen(outp, txt ? "w" : "wb");            /* the log in text mode, as the model's stdout */
    if (!o) { fprintf(stderr, "cannot write %s\n", outp); free(s); return 1; }
    opl2_reset();
    g_log = txt ? o : NULL;
    g_tick = 0;
    start_403(1, s, n);
    free(s);
    unsigned ticks = (unsigned)(seconds * 1193182 / 16383), loops = 0, stopped = 0;
    uint32_t samples = (uint32_t)(seconds * 14318180.0 / 288);
    if (!txt) {
        fwrite("RIFF", 1, 4, o); le(o, 36 + 2 * samples, 4); fwrite("WAVEfmt ", 1, 8, o);
        le(o, 16, 4); le(o, 1, 2); le(o, 1, 2); le(o, OPL2_RATE, 4); le(o, 2 * OPL2_RATE, 4); le(o, 2, 2); le(o, 16, 2);
        fwrite("data", 1, 4, o); le(o, 2 * samples, 4);
    }
    for (uint32_t i = 0; txt ? g_tick < ticks : i < samples; i++) {
        if (txt || (g_acc += 8) >= 5461) {
            if (!txt) g_acc -= 5461;
            g_tick++;
            tick_4A9();
            if (g_loops != loops) { loops = g_loops; fprintf(stderr, "loop at tick %u\n", g_tick); }
            if (!stopped && DB(0x3D1) == 0) { stopped = g_tick; fprintf(stderr, "stopped (FC xx) at tick %u\n", g_tick); }
        }
        if (!txt) le(o, (uint16_t)opl2_sample(), 2);
    }
    g_log = NULL;
    fprintf(stderr, "%u ticks (%.2f s at 72.83 Hz)%s\n", g_tick, g_tick * 16383.0 / 1193182, txt ? "" : ", WAV 49716 Hz mono 16-bit");
    return fclose(o) != 0;
}
