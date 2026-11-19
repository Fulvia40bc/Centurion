#ifndef DRO_H
#define DRO_H

#include <stddef.h>
#include <stdint.h>

typedef struct { uint32_t delay_ms; uint8_t bank, reg, value; } dro_event;  /* delay_ms = wait before this write */
typedef struct { dro_event *ev; uint32_t count; uint32_t length_ms; uint8_t hardware; } dro_file;

int  dro_parse(const uint8_t *data, size_t len, dro_file *out);   /* 0 ok, <0 error code */
void dro_free(dro_file *f);

#endif
