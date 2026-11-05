#ifndef ASSETS_H
#define ASSETS_H
#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t offset;
    uint16_t length;
    char name[14];
    uint8_t format;
} dir_entry_t;

int assets_load(const char *dir_path, const char *dat_path);
int assets_find(const char *name);
uint8_t *assets_decode(int idx, size_t *out_len);

#endif
