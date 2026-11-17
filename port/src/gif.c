#include "gif.h"

#include <stdlib.h>
#include <string.h>

#define GIF_MAX_CODE 4096
#define GIF_MAX_CODE_SIZE 12

static uint16_t rd_u16le(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

typedef struct {
    const uint8_t *data;
    size_t len;
    size_t pos;
    size_t block_left;
} gif_sub;

static int sub_byte(gif_sub *s, uint8_t *out)
{
    if (s->block_left == 0) {
        uint8_t n;
        if (s->pos >= s->len)
            return -1;
        n = s->data[s->pos++];
        if (n == 0)
            return -1;
        s->block_left = n;
    }
    *out = s->data[s->pos++];
    s->block_left--;
    return 0;
}

typedef struct {
    gif_sub sub;
    uint32_t bitbuf;
    int bitcnt;
} gif_bits;

static int bits_read(gif_bits *b, int nbits, uint32_t *out)
{
    while (b->bitcnt < nbits) {
        uint8_t byte;
        if (sub_byte(&b->sub, &byte) != 0)
            return -1;
        b->bitbuf |= (uint32_t)byte << b->bitcnt;
        b->bitcnt += 8;
    }
    *out = b->bitbuf & ((1u << nbits) - 1u);
    b->bitbuf >>= nbits;
    b->bitcnt -= nbits;
    return 0;
}

typedef struct {
    uint8_t *pixels;
    uint32_t w;
    uint32_t h;
    size_t written;
    int interlaced;
    int pass;
    uint32_t x;
    uint32_t y;
} gif_pixels;

static int put_pixel(gif_pixels *p, uint8_t v)
{
    static const uint32_t pass_start[4] = { 0, 4, 2, 1 };
    static const uint32_t pass_step[4] = { 8, 8, 4, 2 };

    if (p->written >= (size_t)p->w * (size_t)p->h)
        return -1;
    p->pixels[(size_t)p->y * p->w + p->x] = v;
    p->written++;
    if (++p->x == p->w) {
        p->x = 0;
        if (!p->interlaced) {
            p->y++;
        } else {
            p->y += pass_step[p->pass];
            if (p->y >= p->h) {
                p->pass++;
                p->y = (p->pass < 4) ? pass_start[p->pass] : p->h;
            }
        }
    }
    return 0;
}

static int lzw_decode(const uint8_t *data, size_t len, size_t pos,
                      uint8_t min_code_size, uint16_t w, uint16_t h,
                      int interlaced, uint8_t *pixels)
{
    gif_bits bits;
    uint16_t prefix[GIF_MAX_CODE];
    uint8_t suffix[GIF_MAX_CODE];
    uint8_t stack[GIF_MAX_CODE];
    uint32_t clear_code, end_code, next, code_size;
    int prev;
    uint8_t first_char;
    gif_pixels px;

    bits.sub.data = data;
    bits.sub.len = len;
    bits.sub.pos = pos;
    bits.sub.block_left = 0;
    bits.bitbuf = 0;
    bits.bitcnt = 0;

    clear_code = 1u << min_code_size;
    end_code = clear_code + 1u;
    next = end_code + 1u;
    code_size = (uint32_t)min_code_size + 1u;
    prev = -1;
    first_char = 0;

    px.pixels = pixels;
    px.w = w;
    px.h = h;
    px.written = 0;
    px.interlaced = interlaced;
    px.pass = 0;
    px.x = 0;
    px.y = 0;

    for (;;) {
        uint32_t code;
        if (bits_read(&bits, (int)code_size, &code) != 0)
            return -4;
        if (code == clear_code) {
            next = end_code + 1u;
            code_size = (uint32_t)min_code_size + 1u;
            prev = -1;
            continue;
        }
        if (code == end_code)
            break;
        if (prev < 0) {
            if (code >= clear_code)
                return -4;
            if (put_pixel(&px, (uint8_t)code) != 0)
                return -4;
            first_char = (uint8_t)code;
            prev = (int)code;
            continue;
        }
        {
            uint32_t c = code;
            int sp = 0;
            if (c >= next) {
                if (c != next)
                    return -4;
                stack[sp++] = first_char;
                c = (uint32_t)prev;
            }
            while (c >= clear_code) {
                if (sp >= GIF_MAX_CODE)
                    return -4;
                stack[sp++] = suffix[c];
                c = prefix[c];
            }
            if (sp >= GIF_MAX_CODE)
                return -4;
            stack[sp++] = (uint8_t)c;
            first_char = (uint8_t)c;
            while (sp > 0) {
                if (put_pixel(&px, stack[--sp]) != 0)
                    return -4;
            }
            if (next < GIF_MAX_CODE) {
                prefix[next] = (uint16_t)prev;
                suffix[next] = first_char;
                next++;
                if (next == (1u << code_size) && code_size < GIF_MAX_CODE_SIZE)
                    code_size++;
            }
            prev = (int)code;
        }
    }
    if (px.written != (size_t)w * (size_t)h)
        return -4;
    return 0;
}

int gif_decode(const uint8_t *data, size_t len, gif_image *out)
{
    size_t pos;
    uint8_t packed;
    int have_gct;
    uint32_t gct_size;

    if (out == NULL)
        return -1;
    memset(out, 0, sizeof *out);
    if (data == NULL || len < 13)
        return -1;
    if (memcmp(data, "GIF87a", 6) != 0 && memcmp(data, "GIF89a", 6) != 0)
        return -1;
    out->screen_w = rd_u16le(data + 6);
    out->screen_h = rd_u16le(data + 8);
    pos = 10;
    packed = data[pos++];
    pos += 2;
    have_gct = (packed & 0x80u) != 0;
    gct_size = 1u << ((packed & 0x07u) + 1u);
    if (have_gct) {
        if ((size_t)gct_size * 3u > len - pos)
            return -2;
        memcpy(out->pal, data + pos, (size_t)gct_size * 3u);
        out->ncolors = (uint16_t)gct_size;
        pos += (size_t)gct_size * 3u;
    }
    for (;;) {
        uint8_t b;
        if (pos >= len)
            return -2;
        b = data[pos++];
        if (b == 0x3Bu)
            return -3;
        if (b == 0x21u) {
            uint8_t n;
            if (pos >= len)
                return -2;
            pos++;
            for (;;) {
                if (pos >= len)
                    return -2;
                n = data[pos++];
                if (n == 0)
                    break;
                if ((size_t)n > len - pos)
                    return -2;
                pos += n;
            }
            continue;
        }
        if (b == 0x2Cu) {
            uint16_t iw, ih;
            uint8_t ipacked, min_code_size;
            int interlaced;
            uint32_t lct_size;
            int rc;

            if (len - pos < 9)
                return -2;
            out->left = rd_u16le(data + pos);
            out->top = rd_u16le(data + pos + 2);
            pos += 4;
            iw = rd_u16le(data + pos);
            pos += 2;
            ih = rd_u16le(data + pos);
            pos += 2;
            ipacked = data[pos++];
            interlaced = (ipacked & 0x40u) != 0;
            lct_size = 1u << ((ipacked & 0x07u) + 1u);
            if ((ipacked & 0x80u) != 0) {
                if ((size_t)lct_size * 3u > len - pos)
                    return -2;
                if (!have_gct) {
                    memcpy(out->pal, data + pos, (size_t)lct_size * 3u);
                    out->ncolors = (uint16_t)lct_size;
                }
                pos += (size_t)lct_size * 3u;
            }
            if (pos >= len)
                return -2;
            min_code_size = data[pos++];
            if (min_code_size < 2 || min_code_size > 8)
                return -4;
            if (iw == 0 || ih == 0 || out->ncolors == 0)
                return -4;
            out->w = iw;
            out->h = ih;
            out->pixels = malloc((size_t)iw * (size_t)ih);
            if (out->pixels == NULL)
                return -5;
            rc = lzw_decode(data, len, pos, min_code_size, iw, ih, interlaced, out->pixels);
            if (rc != 0) {
                free(out->pixels);
                out->pixels = NULL;
                return rc;
            }
            return 0;
        }
        return -3;
    }
}

void gif_free(gif_image *img)
{
    if (img == NULL)
        return;
    free(img->pixels);
    img->pixels = NULL;
    img->w = 0;
    img->h = 0;
    img->ncolors = 0;
}
