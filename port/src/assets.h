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
/* Decode a 0xC0 CBM image member to 8-bit palette indices (w*h bytes).
   Returns NULL on failure. out_w and out_h get the pixel dimensions. */
uint8_t *assets_decode_cbm(int idx, int *out_w, int *out_h);
/* Load the 16x16-byte CBM colour lookup tables from OVL0.OVL (file offset 0x114C0). */
int assets_load_luts(const char *ovl0_path);

#endif
