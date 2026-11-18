#ifndef WAV_H
#define WAV_H

#include <stddef.h>
#include <stdint.h>

typedef struct { uint32_t rate; uint16_t channels; uint32_t frames; int16_t *samples; /* frames*channels, interleaved */ } wav_sound;

int  wav_decode(const uint8_t *data, size_t len, wav_sound *out);   /* 0 ok, <0 error code */
void wav_free(wav_sound *w);

#endif
