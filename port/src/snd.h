#ifndef SND_H
#define SND_H

/* The game's Miles AIL 3.03 calls (port glue, Claude 22 Nov). The game code calls these with the arguments it passes
   in the original (the handles as stored in its data object: 0xF6274 driver, 0xF6278 + 4s samples, 0xF6284 the
   sequence). Until a backend is attached (snd_attach, by main.c through snd_ail.c) every call does nothing and
   returns what the port's no-op stubs returned before: sample status 2 (done), active count 0, sequence status 2,
   set_sample_file 1 when it has bytes, init_sequence 1, allocations 1. So the unit tests link snd.c alone. */

#include <stddef.h>
#include <stdint.h>

typedef struct snd_ops {
    void    (*startup)(void);                          /* AIL_startup (boot_sound 0x10B4D) */
    void    (*shutdown)(void);                         /* AIL_shutdown (0x44D43) */
    int32_t (*allocate_sample)(void);
    int32_t (*allocate_sequence)(void);
    int     (*set_sample_file)(int32_t h, const uint8_t *img, size_t len);
    void    (*set_sample_volume)(int32_t h, int v);
    void    (*set_sample_loop_count)(int32_t h, int n);
    void    (*start_sample)(int32_t h);
    void    (*end_sample)(int32_t h);
    int     (*sample_status)(int32_t h);
    int     (*active_sample_count)(void);
    int     (*init_sequence)(int32_t h, const uint8_t *img, size_t len, int n);
    void    (*start_sequence)(int32_t h);
    void    (*stop_sequence)(int32_t h);
    void    (*resume_sequence)(int32_t h);
    void    (*end_sequence)(int32_t h);
    int     (*sequence_status)(int32_t h);
    void    (*set_sequence_volume)(int32_t h, int v);
    void    (*set_sequence_loop_count)(int32_t h, int n);
    const char *(*last_error)(void);
    const uint8_t *(*music_image)(const char *name, size_t *len);   /* the song the engine plays for name, or NULL */
    char    music_set;                                 /* the set the music engine plays ('g': GM), 0 = as named */
} snd_ops;

void        snd_attach(const snd_ops *ops);            /* NULL detaches */
char        snd_music_set(void);                       /* 0 with no backend */

void        snd_startup(void);
void        snd_shutdown(void);
int32_t     snd_allocate_sample(void);
int32_t     snd_allocate_sequence(void);
int         snd_set_sample_file(uint32_t h, const uint8_t *img, size_t len);   /* 1 ok, 0 failed (the game quits) */
void        snd_set_sample_volume(uint32_t h, int v);
void        snd_set_sample_loop_count(uint32_t h, int n);
void        snd_start_sample(uint32_t h);
void        snd_end_sample(uint32_t h);
int32_t     snd_sample_status(uint32_t h);
int32_t     snd_active_sample_count(uint32_t dig);
int         snd_init_sequence(uint32_t h, const uint8_t *img, size_t len, int n);   /* 1 ok, else the game quits */
void        snd_start_sequence(uint32_t h);
void        snd_stop_sequence(uint32_t h);
void        snd_resume_sequence(uint32_t h);
void        snd_end_sequence(uint32_t h);
int32_t     snd_sequence_status(uint32_t h);
void        snd_set_sequence_volume(uint32_t h, int v, int ms);
void        snd_set_sequence_loop_count(uint32_t h, int n);
const char *snd_last_error(void);                      /* AIL's error text (the original's 0x11AF7C) */
const uint8_t *snd_music_image(const char *name, size_t *len);   /* the engine's own image of a song (the g file for
                                                   GM), NULL: play the game's block (no backend, headless) */

#endif
