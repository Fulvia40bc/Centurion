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

/* Image viewer support: scan all DIR/DAT pairs in a directory for CBM members */
typedef struct {
    char name[14];
    char dat[260];
    uint32_t offset;
    uint16_t length;
} img_entry_t;

int assets_scan_images(const char *dir, img_entry_t *out, int max);
/* Decode a CBM from an explicit dat file; also returns x/y hotspot and transparent nibble. */
uint8_t *assets_decode_cbm_file(const char *dat_path, const img_entry_t *e,
                                int *out_w, int *out_h, int *out_x, int *out_y, int *out_trans);

/* Font (from OVL0.OVL file offset 0x1509C; see docs/FONT_FORMAT.md):
   width[96], yoff[96], glyph[96][8]; chars 0x20..0x7F, 8 rows, bit7 = leftmost. */
int assets_load_font(const char *ovl0_path);
const uint8_t *font_glyph(int ch, int *w, int *yoff);  /* NULL if out of range */
