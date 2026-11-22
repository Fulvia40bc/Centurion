#ifndef AIL_H
#define AIL_H

/* A facade over the port's sound modules (TASK 22), with the handle model of Miles AIL 3.03 that the game calls:
   the driver is handle 1, samples are 1..16 in allocation order, sequences 1..8; handles are small positive
   int32_t, 0 = none. Nothing but the handles is visible to the game. */

#include <stddef.h>
#include <stdint.h>

enum { AIL_MAX_SAMPLES = 16, AIL_MAX_SEQUENCES = 8 };
enum { AIL_FREE = 1, AIL_DONE = 2, AIL_PLAYING = 4, AIL_PAUSED = 8 };

void     ail_init(void (*lock)(int on), const char *sf2_path);   /* lock = plat_audio_lock; tests pass NULL */
void     ail_shutdown(void);                                     /* the caller stopped the audio first */
void     ail_render(int16_t *out, int frames);                    /* what the platform's audio fill calls */
void     ail_set_live(int on);                                   /* 1 while a running device calls ail_render */

int32_t  ail_set_preference(int n, int32_t v);                    /* stores, returns the old value */

int32_t  ail_allocate_sample(void);                              /* first free of 16, or 0 + error text */
void     ail_init_sample(int32_t h);                             /* status DONE, loop count 1, volume 100 */
int      ail_set_sample_file(int32_t h, const uint8_t *img, size_t len);   /* 1 ok, 0 + error text */
void     ail_set_sample_volume(int32_t h, int v);                /* 0..127 clamped */
void     ail_set_sample_loop_count(int32_t h, int n);
void     ail_start_sample(int32_t h);                            /* always from the start; status PLAYING */
void     ail_end_sample(int32_t h);                              /* status DONE unless free or done; voice stops */
int      ail_sample_status(int32_t h);                           /* 0 unknown, else 1/2/4 */
int      ail_active_sample_count(void);                          /* handles with status PLAYING */

int32_t  ail_allocate_sequence(void);                            /* first free of 8, status DONE */
int      ail_init_sequence(int32_t h, const uint8_t *img, size_t len, int n);  /* 1 ok, 0 + error text */
void     ail_start_sequence(int32_t h);                          /* rewind and play; status PLAYING */
void     ail_stop_sequence(int32_t h);                           /* only when playing: status PAUSED */
void     ail_resume_sequence(int32_t h);                         /* only when paused: status PLAYING */
void     ail_end_sequence(int32_t h);                            /* stop the notes; status DONE */
int      ail_sequence_status(int32_t h);                         /* 0 unknown, else 1/2/4/8 */
void     ail_set_sequence_volume(int32_t h, int v, int ms);      /* ms is always 0 in the game; any ms = 0 */
void     ail_set_sequence_loop_count(int32_t h, int n);

const char *ail_last_error(void);                                /* the last error text (the game prints it) */

#endif
