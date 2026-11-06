#include "assets.h"
#include "compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <dirent.h>
#else
#include <windows.h>
typedef struct { HANDLE h; WIN32_FIND_DATAA d; int first; } DIR;
typedef struct { char d_name[260]; } dirent_compat;
#define dirent dirent_compat
static DIR *opendir(const char *p) {
    DIR *d = malloc(sizeof(DIR));
    d->h = FindFirstFileA(p, &d->d);
    d->first = (d->h != INVALID_HANDLE_VALUE);
    return d->h == INVALID_HANDLE_VALUE ? NULL : d;
}
static struct dirent *readdir(DIR *d) {
    static dirent_compat e;
    if (!d->first && !FindNextFileA(d->h, &d->d)) return NULL;
    strncpy(e.d_name, d->d.cFileName, 259); e.d_name[259] = 0;
    d->first = 0;
    return &e;
}
static void closedir(DIR *d) { if (d->h != INVALID_HANDLE_VALUE) FindClose(d->h); free(d); }
#endif

static uint8_t *g_dat;
static long g_dat_len;
static dir_entry_t g_entries[256];
static int g_entry_count;

int assets_load(const char *dir_path, const char *dat_path) {
    FILE *f = fopen(dir_path, "rb");
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    long dir_len = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *dir = malloc(dir_len);
    if (fread(dir, 1, dir_len, f) != (size_t)dir_len) { fclose(f); free(dir); return -1; }
    fclose(f);
    g_entry_count = 0;
    for (long off = 0; off + 20 <= dir_len && g_entry_count < 256; off += 20) {
        dir_entry_t *e = &g_entries[g_entry_count];
        e->offset = dir[off] | (dir[off+1] << 8) | ((uint32_t)dir[off+2] << 16) | ((uint32_t)dir[off+3] << 24);
        e->length = dir[off+4] | (dir[off+5] << 8);
        memcpy(e->name, dir + off + 6, 13);
        e->name[13] = 0;
        for (int i = 0; i < 13; i++) if (e->name[i] == 0) { e->name[i] = 0; break; }
        e->format = dir[off+19];
        g_entry_count++;
    }
    free(dir);

    f = fopen(dat_path, "rb");
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    g_dat_len = ftell(f);
    fseek(f, 0, SEEK_SET);
    g_dat = malloc(g_dat_len);
    if (fread(g_dat, 1, g_dat_len, f) != (size_t)g_dat_len) { fclose(f); return -1; }
    fclose(f);
    if (g_dat_len < 4 || g_dat[0] != 0xCA || g_dat[1] != 0xCA || g_dat[2] != 0xD0 || g_dat[3] != 0xD0)
        return -1;
    return 0;
}

int assets_find(const char *name) {
    for (int i = 0; i < g_entry_count; i++)
        if (strcasecmp(g_entries[i].name, name) == 0) return i;
    return -1;
}

/* LZSS as used by the game's unlzss: 4KB window prefilled 0x20, wpos starts 0xFEE,
   flag byte LSB-first (1=literal, 0=match), match = 2 bytes: 12-bit pos + len-3 */
uint8_t *assets_decode(int idx, size_t *out_len) {
    if (idx < 0) return NULL;
    dir_entry_t *e = &g_entries[idx];
    const uint8_t *src = g_dat + e->offset;   /* DIR offsets are absolute (incl. 4-byte sig) */
    if (e->format != 0x80) {
        uint8_t *out = malloc(e->length);
        memcpy(out, src, e->length);
        *out_len = e->length;
        return out;
    }
    /* 0x80 members: uint32 uncompressed size, then the LZSS stream */
    uint32_t usize = (uint32_t)src[0] | ((uint32_t)src[1] << 8) |
                     ((uint32_t)src[2] << 16) | ((uint32_t)src[3] << 24);
    size_t cap = usize + 16;
    uint8_t *out = malloc(cap);
    uint8_t window[4096];
    memset(window, 0x20, sizeof(window));
    size_t wpos = 0xFEE, olen = 0, i = 4;
    while (i < e->length && olen < cap) {
        uint8_t flags = src[i++];
        for (int bit = 0; bit < 8 && i < e->length; bit++) {
            if (flags & (1 << bit)) {
                uint8_t b = src[i++];
                out[olen++] = b;
                window[wpos] = b;
                wpos = (wpos + 1) & 0xFFF;
            } else {
                if (i + 1 >= e->length) break;
                uint8_t b1 = src[i], b2 = src[i+1];
                i += 2;
                size_t pos = (b1 | ((b2 & 0xF0) << 4)) & 0xFFF;
                int ln = (b2 & 0x0F) + 3;
                for (int k = 0; k < ln && olen < cap; k++) {
                    uint8_t b = window[(pos + k) & 0xFFF];
                    out[olen++] = b;
                    window[wpos] = b;
                    wpos = (wpos + 1) & 0xFFF;
                }
            }
        }
    }
    if (olen > usize) olen = usize;   /* truncate to declared size */
    *out_len = olen;
    return out;
}


static uint8_t g_luts[64 * 16];   /* up to 64 tables x 16 bytes, from OVL0.OVL offset 0x114C0 */
static int g_luts_loaded;

int assets_load_luts(const char *ovl0_path) {
    FILE *f = fopen(ovl0_path, "rb");
    if (!f) return -1;
    if (fseek(f, 0x114C0, SEEK_SET) != 0 || fread(g_luts, 1, sizeof(g_luts), f) != sizeof(g_luts)) {
        fclose(f);
        return -1;
    }
    fclose(f);
    g_luts_loaded = 1;
    return 0;
}

/* CBM image member (format 0xC0): 14-byte header then an LZSS stream.
   header: u16 width_px, s16 x, s16 y, u8 bytes_per_row, u8 rows, u8 transparent,
   u8 lut_index, u16 unpacked_size, u16 pad. Payload = LZSS -> 4-bit packed pixels
   (hi nibble first); each nibble maps through the 16-entry LUT at header byte 9. */
uint8_t *assets_decode_cbm(int idx, int *out_w, int *out_h, int *out_lut) {
    if (idx < 0 || g_entries[idx].format != 0xC0 || !g_luts_loaded) return NULL;
    dir_entry_t *e = &g_entries[idx];
    const uint8_t *src = g_dat + e->offset;
    uint16_t w = (uint16_t)(src[0] | (src[1] << 8));
    uint8_t rows = src[7];
    uint8_t lut = src[9];
    if (lut >= 64) lut = 0;   /* bounds-check: CBMs use LUT indices up to 29 */
    uint16_t usize = (uint16_t)(src[10] | (src[11] << 8));
    size_t cap = usize + 16;
    uint8_t *packed = malloc(cap);
    uint8_t window[4096];
    memset(window, 0x20, sizeof(window));
    size_t wpos = 0xFEE, olen = 0, i = 14;
    while (i < e->length && olen < cap) {
        uint8_t flags = src[i++];
        for (int bit = 0; bit < 8 && i < e->length; bit++) {
            if (flags & (1 << bit)) {
                uint8_t b = src[i++];
                packed[olen++] = b;
                window[wpos] = b;
                wpos = (wpos + 1) & 0xFFF;
            } else {
                if (i + 1 >= e->length) break;
                uint8_t b1 = src[i], b2 = src[i+1];
                i += 2;
                size_t pos = (b1 | ((b2 & 0xF0) << 4)) & 0xFFF;
                int ln = (b2 & 0x0F) + 3;
                for (int k = 0; k < ln && olen < cap; k++) {
                    uint8_t b = window[(pos + k) & 0xFFF];
                    packed[olen++] = b;
                    window[wpos] = b;
                    wpos = (wpos + 1) & 0xFFF;
                }
            }
        }
    }
    if (olen > usize) olen = usize;
    /* Rows are stored bytes_per_row (src[6]) bytes = 2*bpr nibbles wide; for odd widths
       (TITLE.CBM: 313 px, 157 bytes/row) the last nibble of each row is padding. Crop to w.
       VERIFIED (Claude): native output now matches the reference decoder for all 82 CBMs. */
    size_t bpr = src[6];
    uint8_t *px = calloc((size_t)w * rows + 1, 1);
    for (size_t y = 0; y < rows; y++)
        for (size_t x = 0; x < (size_t)w; x++) {
            size_t k = y * bpr + x / 2;
            if (k >= olen) continue;
            uint8_t b = packed[k];
            px[y * w + x] = (x & 1) ? (b & 0xF) : ((b >> 4) & 0xF);   /* raw nibbles; caller maps via LUT */
        }
    free(packed);
    *out_w = (int)w;
    *out_h = (int)rows;
    if (out_lut) *out_lut = lut;
    return px;
}

/* ---- image viewer support ---- */

static size_t lzss_run(const uint8_t *src, size_t srclen, size_t start,
                       uint8_t *out, size_t cap) {
    uint8_t window[4096];
    memset(window, 0x20, sizeof(window));
    size_t wpos = 0xFEE, olen = 0, i = start;
    while (i < srclen && olen < cap) {
        uint8_t flags = src[i++];
        for (int bit = 0; bit < 8 && i < srclen; bit++) {
            if (flags & (1 << bit)) {
                uint8_t b = src[i++];
                out[olen++] = b;
                window[wpos] = b;
                wpos = (wpos + 1) & 0xFFF;
            } else {
                if (i + 1 >= srclen) break;
                uint8_t b1 = src[i], b2 = src[i+1];
                i += 2;
                size_t pos = (b1 | ((b2 & 0xF0) << 4)) & 0xFFF;
                int ln = (b2 & 0x0F) + 3;
                for (int k = 0; k < ln && olen < cap; k++) {
                    uint8_t b = window[(pos + k) & 0xFFF];
                    out[olen++] = b;
                    window[wpos] = b;
                    wpos = (wpos + 1) & 0xFFF;
                }
            }
        }
    }
    return olen;
}

int assets_scan_images(const char *dir, img_entry_t *out, int max) {
    DIR *d = opendir(dir);
    if (!d) return -1;
    int n = 0;
    struct dirent *de;
    while ((de = readdir(d)) && n < max) {
        size_t l = strlen(de->d_name);
        if (l < 5 || strcasecmp(de->d_name + l - 4, ".dir") != 0) continue;
        char dirpath[512], datpath[512];
        snprintf(dirpath, sizeof(dirpath), "%s/%s", dir, de->d_name);
        snprintf(datpath, sizeof(datpath), "%s/%.*s.DAT", dir, (int)(l - 4), de->d_name);
        FILE *f = fopen(dirpath, "rb");
        if (!f) continue;
        fseek(f, 0, SEEK_END);
        long dl = ftell(f);
        fseek(f, 0, SEEK_SET);
        uint8_t *buf = malloc(dl);
        if (fread(buf, 1, dl, f) != (size_t)dl) { fclose(f); free(buf); continue; }
        fclose(f);
        for (long off = 0; off + 20 <= dl; off += 20) {
            if (buf[off + 19] != 0xC0) continue;
            if (n >= max) break;
            img_entry_t *e = &out[n++];
            e->offset = (uint32_t)buf[off] | ((uint32_t)buf[off+1] << 8) |
                        ((uint32_t)buf[off+2] << 16) | ((uint32_t)buf[off+3] << 24);
            e->length = (uint16_t)(buf[off+4] | (buf[off+5] << 8));
            memcpy(e->name, buf + off + 6, 13);
            e->name[13] = 0;
            for (int i = 0; i < 13; i++) if (e->name[i] == 0) break;
            snprintf(e->dat, sizeof(e->dat), "%s", datpath);
        }
        free(buf);
    }
    closedir(d);
    return n;
}

uint8_t *assets_decode_cbm_file(const char *dat_path, const img_entry_t *e,
                                int *out_w, int *out_h, int *out_x, int *out_y, int *out_trans, int *out_lut) {
    FILE *f = fopen(dat_path, "rb");
    if (!f || !g_luts_loaded) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *dat = malloc(len);
    if (fread(dat, 1, len, f) != (size_t)len) { fclose(f); free(dat); return NULL; }
    fclose(f);
    const uint8_t *src = dat + e->offset;
    int w = src[0] | (src[1] << 8);
    int xo = (int16_t)(src[2] | (src[3] << 8));
    int yo = (int16_t)(src[4] | (src[5] << 8));
    uint8_t rows = src[7];
    uint8_t trans = src[8];
    uint8_t lut = src[9];
    if (lut >= 64) lut = 0;   /* bounds-check: CBMs use LUT indices up to 29 */
    uint16_t usize = (uint16_t)(src[10] | (src[11] << 8));
    uint8_t *packed = malloc(usize + 16);
    size_t olen = lzss_run(src, e->length, 14, packed, usize + 16);
    if (olen > usize) olen = usize;
    /* Rows are stored bytes_per_row (src[6]) bytes = 2*bpr nibbles wide; for odd widths
       (TITLE.CBM: 313 px, 157 bytes/row) the last nibble of each row is padding. Crop to w.
       VERIFIED (Claude): native output now matches the reference decoder for all 82 CBMs. */
    size_t bpr = src[6];
    uint8_t *px = calloc((size_t)w * rows + 1, 1);
    for (size_t y = 0; y < rows; y++)
        for (size_t x = 0; x < (size_t)w; x++) {
            size_t k = y * bpr + x / 2;
            if (k >= olen) continue;
            uint8_t b = packed[k];
            px[y * w + x] = (x & 1) ? (b & 0xF) : ((b >> 4) & 0xF);   /* raw nibbles; caller maps via LUT */
        }
    free(packed);
    free(dat);
    *out_w = w; *out_h = (int)rows; *out_x = xo; *out_y = yo; *out_trans = trans;
    if (out_lut) *out_lut = lut;
    return px;
}

static int cbm_lut_byte(const img_entry_t *e) {
    FILE *f = fopen(e->dat, "rb");
    if (!f) return -1;
    int v = -1;
    if (fseek(f, (long)e->offset + 9, SEEK_SET) == 0) v = fgetc(f);
    fclose(f);
    return v;
}

int assets_effective_lut(const img_entry_t *imgs, int i) {
    for (int j = i; j >= 0 && strcmp(imgs[j].dat, imgs[i].dat) == 0; j--) {
        int v = cbm_lut_byte(&imgs[j]);
        if (v > 0 && v < 64) return v;
    }
    return 0;
}

const uint8_t *assets_lut(int idx) {
    if (!g_luts_loaded || idx < 0 || idx >= 64) return NULL;
    return g_luts + 16 * idx;
}

/* ---- font ---- */
static uint8_t g_font[96 + 96 + 96 * 8];
static int g_font_loaded;

int assets_load_font(const char *ovl0_path) {
    FILE *f = fopen(ovl0_path, "rb");
    if (!f) return -1;
    if (fseek(f, 0x1509C, SEEK_SET) != 0 || fread(g_font, 1, sizeof(g_font), f) != sizeof(g_font)) {
        fclose(f);
        return -1;
    }
    fclose(f);
    g_font_loaded = 1;
    return 0;
}

const uint8_t *font_glyph(int ch, int *w, int *yoff) {
    if (!g_font_loaded || ch < 0x20 || ch > 0x7F) return NULL;
    int c = ch - 0x20;
    *w = g_font[c];
    *yoff = g_font[96 + c];
    return g_font + 192 + c * 8;
}
