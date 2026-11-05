#include "assets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    const uint8_t *src = g_dat + 4 + e->offset;
    size_t cap = e->length * 8 + 4096;
    uint8_t *out = malloc(cap);
    uint8_t window[4096];
    memset(window, 0x20, sizeof(window));
    size_t wpos = 0xFEE, olen = 0, i = 0;
    if (e->format != 0x80) {
        memcpy(out, src, e->length);
        *out_len = e->length;
        return out;
    }
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
    *out_len = olen;
    return out;
}
