#include "le_image.h"

#include <stdlib.h>
#include <string.h>

static uint32_t u16(const uint8_t *d, size_t o) { return (uint32_t)d[o] | (uint32_t)d[o + 1] << 8; }
static uint32_t u32(const uint8_t *d, size_t o) { return u16(d, o) | u16(d, o + 2) << 16; }
static int32_t  s16(const uint8_t *d, size_t o) { uint32_t v = u16(d, o); return v & 0x8000u ? (int32_t)v - 0x10000 : (int32_t)v; }
static void put32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }

/* the last MZ in the file whose e_lfanew points at "LE": the game's own stub, not the extender's */
static int find_le(const uint8_t *d, size_t len, size_t *mz, size_t *le)
{
    size_t o;
    int found = 0;

    for (o = 0; o + 0x40 <= len; o++) {
        uint32_t lf;
        if (d[o] != 'M' || d[o + 1] != 'Z')
            continue;
        lf = u32(d, o + 0x3C);
        if (o + lf + 2 <= len && d[o + lf] == 'L' && d[o + lf + 1] == 'E') {
            *mz = o;
            *le = o + lf;
            found = 1;
        }
    }
    return found;
}

int le_load_object(const uint8_t *d, size_t len, int n, le_object *out)
{
    size_t mz, le, data_base;
    uint32_t npages, page_size, last_page, obj_tab, nobj, page_map, fix_page_tab, fix_rec_tab, data_pages;
    uint32_t base[8], pidx, pcnt, k, pn;

    memset(out, 0, sizeof *out);
    if (d == NULL || !find_le(d, len, &mz, &le) || le + 0x84 > len)
        return -1;
    npages = u32(d, le + 0x14);
    page_size = u32(d, le + 0x28);
    last_page = u32(d, le + 0x2C);
    obj_tab = u32(d, le + 0x40);
    nobj = u32(d, le + 0x44);
    page_map = u32(d, le + 0x48);
    fix_page_tab = u32(d, le + 0x68);
    fix_rec_tab = u32(d, le + 0x6C);
    data_pages = u32(d, le + 0x80);
    if (page_size == 0 || nobj == 0 || nobj > 8)
        return -1;
    /* data pages are relative to the file for a plain LE, to the stub MZ for a bound DOS/4GW program */
    data_base = (data_pages > le && data_pages + (size_t)(npages - 1) * page_size + last_page <= len) ? data_pages
                                                                                                       : mz + data_pages;
    if (n < 1 || (uint32_t)n > nobj)
        return -2;
    for (k = 0; k < nobj; k++)
        base[k] = u32(d, le + obj_tab + 24 * k + 4);
    {
        size_t e = le + obj_tab + 24 * (size_t)(n - 1);
        out->size = u32(d, e);
        out->base = u32(d, e + 4);
        pidx = u32(d, e + 12);
        pcnt = u32(d, e + 16);
    }
    out->bytes = calloc((size_t)out->size + page_size, 1);   /* + a page: fixups may sit at the very end */
    if (out->bytes == NULL)
        return -4;
    for (k = 0; k < pcnt; k++) {
        size_t e, off, ln;
        uint32_t num;
        pn = pidx + k;
        e = le + page_map + 4 * (size_t)(pn - 1);
        num = (uint32_t)d[e] << 16 | (uint32_t)d[e + 1] << 8 | d[e + 2];
        off = data_base + (size_t)(num - 1) * page_size;
        ln = pn == npages ? last_page : page_size;
        if (off + ln > len || (size_t)k * page_size + ln > (size_t)out->size + page_size) {
            le_free_object(out);
            return -1;
        }
        memcpy(out->bytes + (size_t)k * page_size, d + off, ln);
    }
    for (k = 0; k < pcnt; k++) {   /* the fixup records of this object's pages */
        size_t a, r, rend;
        pn = pidx + k;
        a = le + fix_page_tab + 4 * (size_t)(pn - 1);
        r = le + fix_rec_tab + u32(d, a);
        rend = le + fix_rec_tab + u32(d, a + 4);
        while (r < rend) {
            uint32_t stype = d[r], flags = d[r + 1], cnt = 1, tobj, toff = 0, t, j;
            int32_t first = 0;
            int multi = (stype & 0x20) != 0;
            size_t list = 0;
            r += 2;
            if (multi) {
                cnt = d[r];
                r += 1;
            } else {
                first = s16(d, r);
                r += 2;
            }
            if (flags & 3) {
                le_free_object(out);
                return -3;   /* an import: a flat Watcom program has none */
            }
            if (flags & 0x40) {
                tobj = u16(d, r);
                r += 2;
            } else {
                tobj = d[r];
                r += 1;
            }
            t = stype & 0x0F;
            if (t != 2) {
                if (flags & 0x10) {
                    toff = u32(d, r);
                    r += 4;
                } else {
                    toff = u16(d, r);
                    r += 2;
                }
            }
            if (multi) {
                list = r;
                r += 2 * (size_t)cnt;
            }
            if (tobj < 1 || tobj > nobj) {
                le_free_object(out);
                return -3;
            }
            for (j = 0; j < cnt; j++) {
                int32_t so = multi ? s16(d, list + 2 * (size_t)j) : first;
                int64_t pos = (int64_t)k * page_size + so;
                uint32_t tgt = base[tobj - 1] + toff, saddr = out->base + (uint32_t)pos;
                if (pos < 0 || pos + 4 > (int64_t)out->size + page_size)
                    continue;
                if (t == 7)
                    put32(out->bytes + pos, tgt);                       /* 32-bit offset */
                else if (t == 8)
                    put32(out->bytes + pos, tgt - (saddr + 4));         /* 32-bit self-relative */
                else if (t == 5) {
                    out->bytes[pos] = (uint8_t)tgt;                     /* 16-bit offset */
                    out->bytes[pos + 1] = (uint8_t)(tgt >> 8);
                } else if (t != 2) {
                    le_free_object(out);
                    return -3;
                }
            }
        }
    }
    return 0;
}

void le_free_object(le_object *o)
{
    free(o->bytes);
    memset(o, 0, sizeof *o);
}
