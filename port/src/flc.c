#include "flc.h"

#include <stdlib.h>
#include <string.h>

#define FLC_MAGIC_FLC 0xAF12u
#define FLC_MAGIC_FLI 0xAF11u
#define FLC_FRAME 0xF1FAu
#define FLC_PREFIX 0xF100u

#define CH_COLOR_256 4u
#define CH_DELTA_FLC 7u
#define CH_COLOR_64 11u
#define CH_LC 12u
#define CH_BLACK 13u
#define CH_BRUN 15u
#define CH_COPY 16u

static uint16_t rd_u16le(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t rd_u32le(const uint8_t *p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

static size_t first_frame_pos(const flc_anim *a)
{
    uint32_t p = rd_u32le(a->data + 80);
    return p != 0 ? (size_t)p : 128;
}

static int chunk_color(flc_anim *a, const uint8_t *p, size_t avail, int shift)
{
    uint16_t packets;
    size_t pos = 0;
    unsigned pal_ptr = 0;

    if (avail < 2)
        return -2;
    packets = rd_u16le(p);
    pos = 2;
    while (packets-- > 0) {
        unsigned skip, cnt, j;
        if (pos + 2 > avail)
            return -3;
        skip = p[pos++];
        cnt = p[pos++];
        if (cnt == 0)
            cnt = 256;
        pal_ptr += skip;
        if (pos + (size_t)cnt * 3 > avail)
            return -3;
        for (j = 0; j < cnt; j++) {
            if (pal_ptr >= 256)
                pal_ptr = 0;
            a->pal[pal_ptr][0] = (uint8_t)(p[pos] << shift);
            a->pal[pal_ptr][1] = (uint8_t)(p[pos + 1] << shift);
            a->pal[pal_ptr][2] = (uint8_t)(p[pos + 2] << shift);
            pal_ptr++;
            pos += 3;
        }
    }
    return 0;
}

static int chunk_brun(flc_anim *a, const uint8_t *p, size_t avail)
{
    uint32_t y;
    size_t pos = 0;

    for (y = 0; y < a->h; y++) {
        unsigned packets;
        uint32_t x = 0;
        if (pos >= avail)
            return -2;
        packets = p[pos++];
        while (packets-- > 0 && x < a->w) {
            int c;
            if (pos >= avail)
                return -2;
            c = (int8_t)p[pos++];
            if (c > 0) {
                uint8_t v;
                if (pos >= avail || (uint32_t)c > a->w - x)
                    return -3;
                v = p[pos++];
                memset(a->pixels + (size_t)y * a->w + x, v, (size_t)c);
                x += (uint32_t)c;
            } else if (c < 0) {
                size_t n = (size_t)(-c);
                if (pos + n > avail || (uint32_t)n > a->w - x)
                    return -3;
                memcpy(a->pixels + (size_t)y * a->w + x, p + pos, n);
                pos += n;
                x += (uint32_t)n;
            } else {
                return -4;
            }
        }
    }
    return 0;
}

static int chunk_lc(flc_anim *a, const uint8_t *p, size_t avail)
{
    size_t pos = 0;
    uint16_t start_line, lines;
    uint32_t y;

    if (avail < 4)
        return -2;
    start_line = rd_u16le(p);
    lines = rd_u16le(p + 2);
    pos = 4;
    if (start_line >= a->h)
        return -3;
    y = start_line;
    while (lines-- > 0 && y < a->h) {
        unsigned packets;
        uint32_t x = 0;
        if (pos >= avail)
            return -2;
        packets = p[pos++];
        while (packets-- > 0 && x < a->w) {
            unsigned skip;
            int c;
            if (pos + 2 > avail)
                return -2;
            skip = p[pos++];
            c = (int8_t)p[pos++];
            if (x + skip > a->w)
                return -4;
            x += skip;
            if (c > 0) {
                if (pos + (size_t)c > avail || (uint32_t)c > a->w - x)
                    return -3;
                memcpy(a->pixels + (size_t)y * a->w + x, p + pos, (size_t)c);
                pos += (size_t)c;
                x += (uint32_t)c;
            } else if (c < 0) {
                size_t n = (size_t)(-c);
                uint8_t v;
                if (pos >= avail || (uint32_t)n > a->w - x)
                    return -3;
                v = p[pos++];
                memset(a->pixels + (size_t)y * a->w + x, v, n);
                x += (uint32_t)n;
            }
        }
        y++;
    }
    return 0;
}

static int chunk_delta_flc(flc_anim *a, const uint8_t *p, size_t avail)
{
    size_t pos = 0;
    uint16_t lines;
    uint32_t y = 0;

    if (avail < 2)
        return -2;
    lines = rd_u16le(p);
    pos = 2;
    while (lines > 0) {
        int lp;
        if (pos + 2 > avail)
            return -2;
        lp = (int16_t)rd_u16le(p + pos);
        pos += 2;
        if ((lp & 0xC000) == 0xC000) {
            int skip = -lp;
            if (skip < 0 || (uint32_t)skip > a->h)
                return -3;
            y += (uint32_t)skip;
        } else if ((lp & 0xC000) == 0x8000) {
            if (y < a->h && a->w > 0)
                a->pixels[(size_t)y * a->w + a->w - 1] = (uint8_t)(lp & 0xFF);
        } else {
            unsigned packets = (unsigned)lp;
            uint32_t x = 0;
            lines--;
            if (y >= a->h)
                return -3;
            while (packets-- > 0 && x < a->w) {
                unsigned skip;
                int c;
                if (pos + 2 > avail)
                    return -2;
                skip = p[pos++];
                c = (int8_t)p[pos++];
                if (x + skip > a->w)
                    return -4;
                x += skip;
                if (c < 0) {
                    size_t n = (size_t)(-c);
                    uint8_t b1, b2;
                    if (pos + 2 > avail || (uint32_t)(2 * n) > a->w - x)
                        return -3;
                    b1 = p[pos++];
                    b2 = p[pos++];
                    while (n-- > 0) {
                        a->pixels[(size_t)y * a->w + x] = b1;
                        a->pixels[(size_t)y * a->w + x + 1] = b2;
                        x += 2;
                    }
                } else if (c > 0) {
                    size_t n = (size_t)c * 2;
                    if (pos + n > avail || (uint32_t)n > a->w - x)
                        return -3;
                    memcpy(a->pixels + (size_t)y * a->w + x, p + pos, n);
                    pos += n;
                    x += (uint32_t)n;
                }
            }
            y++;
        }
    }
    return 0;
}

static int chunk_copy(flc_anim *a, const uint8_t *p, size_t avail)
{
    uint32_t row_bytes = ((uint32_t)a->w + 3u) & ~3u;
    uint32_t y;

    if ((size_t)row_bytes * a->h > avail)
        return -3;
    for (y = 0; y < a->h; y++)
        memcpy(a->pixels + (size_t)y * a->w, p + (size_t)y * row_bytes, a->w);
    return 0;
}

static int decode_frame_chunk(flc_anim *a, size_t pos)
{
    uint32_t frame_size;
    uint16_t num_chunks, type;
    size_t p, end;
    unsigned i;
    int rc;

    if (pos + 16 > a->len)
        return -2;
    frame_size = rd_u32le(a->data + pos);
    type = rd_u16le(a->data + pos + 4);
    num_chunks = rd_u16le(a->data + pos + 6);
    if (type != FLC_FRAME || frame_size < 16)
        return -3;
    if ((size_t)frame_size > a->len - pos)
        frame_size = (uint32_t)(a->len - pos);
    p = pos + 16;
    end = pos + frame_size;
    for (i = 0; i < num_chunks && p + 6 <= end; i++) {
        uint32_t csize = rd_u32le(a->data + p);
        uint16_t ctype = rd_u16le(a->data + p + 4);
        const uint8_t *body;
        size_t bavail;
        if (csize < 6)
            return -3;
        if ((size_t)csize > end - p)
            csize = (uint32_t)(end - p);
        body = a->data + p + 6;
        bavail = csize - 6;
        switch (ctype) {
        case CH_COLOR_256:
            rc = chunk_color(a, body, bavail, 0);
            break;
        case CH_COLOR_64:
            rc = chunk_color(a, body, bavail, 2);
            break;
        case CH_DELTA_FLC:
            rc = chunk_delta_flc(a, body, bavail);
            break;
        case CH_LC:
            rc = chunk_lc(a, body, bavail);
            break;
        case CH_BLACK:
            memset(a->pixels, 0, (size_t)a->w * a->h);
            rc = 0;
            break;
        case CH_BRUN:
            rc = chunk_brun(a, body, bavail);
            break;
        case CH_COPY:
            rc = chunk_copy(a, body, bavail);
            break;
        default:
            rc = 0;
            break;
        }
        if (rc != 0)
            return rc;
        p += csize;
    }
    return 0;
}

int flc_open(flc_anim *a, const uint8_t *data, size_t len)
{
    uint16_t magic, depth;

    if (a == NULL)
        return -1;
    memset(a, 0, sizeof *a);
    if (data == NULL || len < 128)
        return -1;
    magic = rd_u16le(data + 4);
    if (magic != FLC_MAGIC_FLC && magic != FLC_MAGIC_FLI)
        return -1;
    a->frames = rd_u16le(data + 6);
    a->w = rd_u16le(data + 8);
    a->h = rd_u16le(data + 10);
    if (a->w == 0)
        a->w = 320;
    if (a->h == 0)
        a->h = 200;
    depth = rd_u16le(data + 12);
    if (depth != 0 && depth != 8)
        return -1;
    if (magic == FLC_MAGIC_FLC)
        a->speed_ms = rd_u32le(data + 16);
    else
        a->speed_ms = (uint32_t)rd_u16le(data + 16) * 1000u / 70u;
    if (a->frames == 0)
        a->frames = 1;
    a->pixels = calloc((size_t)a->w * a->h, 1);
    if (a->pixels == NULL)
        return -5;
    a->data = data;
    a->len = len;
    a->pos = first_frame_pos(a);
    a->index = 0;
    return 0;
}

int flc_next(flc_anim *a)
{
    size_t first;
    int ring, was_first;
    int rc;

    if (a == NULL || a->data == NULL || a->pixels == NULL)
        return -1;
    first = first_frame_pos(a);
    ring = (a->index == (uint32_t)(a->frames - 1)) && (a->pos != first);
    was_first = (a->pos == first);
    while (a->pos + 6 <= a->len && rd_u16le(a->data + a->pos + 4) == FLC_PREFIX)
        a->pos += rd_u32le(a->data + a->pos);
    if (a->pos + 6 > a->len || rd_u16le(a->data + a->pos + 4) != FLC_FRAME) {
        if (ring) {
            a->pos = first;
            a->index = 0;
            return 0;
        }
        return -3;
    }
    rc = decode_frame_chunk(a, a->pos);
    if (rc != 0)
        return rc;
    a->pos += rd_u32le(a->data + a->pos);
    if (ring) {
        a->pos = first;
        a->index = 0;
    } else if (!was_first && a->index < (uint32_t)(a->frames - 1)) {
        a->index++;
    }
    return 0;
}

void flc_close(flc_anim *a)
{
    if (a == NULL)
        return;
    free(a->pixels);
    a->pixels = NULL;
    a->data = NULL;
    a->len = 0;
    a->pos = 0;
    a->index = 0;
    a->w = a->h = a->frames = 0;
    a->speed_ms = 0;
}
