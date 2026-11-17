#ifndef MUSIC_H
#define MUSIC_H
/* The music (music.c): the AdLib driver ADLIB.LZW and the game's calls into it (EXE segment 0823), docs/MUSIC_NOTES.md.
 * Without an audio device (headless runs, nosnd) only the DS parts run: the slot table DS:A6B0 and [3EEE]. */
#include <stdint.h>

void music_load_5D73(uint8_t m);        /* 5D73: song m into a free slot of DS:A6B0 (5EB5) */
void music_play_5D29(uint8_t m);        /* 5D29: 5D73(m), then, unless m plays already, start it (072E) */
void music_stop_5D20(void);             /* 5D20: [3EEE] = FF, the driver's stop (074B) */
void music_unload_5E1A(uint8_t n);      /* 5E1A: the slot of song n freed (the driver is not stopped) */
int  music_init_5E86(void);             /* a window game: ADLIB.LZW, the audio device, 5E86's DS part; 0 = the music runs */
void music_nosnd_B0E0(void);            /* the command-line word "nosnd": [3EEA] = FF, then 5E86's [3EE9] = 0 */
int  music_on(void);                    /* the driver runs (an audio device is open) */
void music_ctrl_s_03FF(void);           /* INT 9's Ctrl+S: [3EE9] ^= FF (the ISR then mutes / unmutes) */
int  music_wav(const char *song, const char *out, double seconds);   /* --music-wav: headless, no SDL */

#endif
