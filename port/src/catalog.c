#include "catalog.h"

#include <string.h>

static uint32_t rd_u32le(const uint8_t *p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

static int is_end_mark(const uint8_t *name)
{
    int i;
    for (i = 0; i < 12; i++)
        if (name[i] != 0xFFu)
            return 0;
    return 1;
}

static int name_eq_ci(const char *a, const char *b)
{
    for (;;) {
        unsigned char ca = (unsigned char)*a++;
        unsigned char cb = (unsigned char)*b++;
        if (ca >= 'a' && ca <= 'z')
            ca = (unsigned char)(ca - 'a' + 'A');
        if (cb >= 'a' && cb <= 'z')
            cb = (unsigned char)(cb - 'a' + 'A');
        if (ca != cb)
            return 0;
        if (ca == 0)
            return 1;
    }
}

int cat_open(catalog *c, const uint8_t *data, size_t size)
{
    uint32_t first;
    size_t count, i;
    int n = 0;

    if (c == NULL)
        return -1;
    c->data = NULL;
    c->size = 0;
    c->count = 0;
    if (data == NULL || size < 24)
        return -1;
    first = rd_u32le(data + 12);
    count = first / 24;
    if (count > 64 || count > size / 24)
        return -1;
    for (i = 0; i < count; i++) {
        const uint8_t *raw = data + 24 * i;
        uint32_t off = rd_u32le(raw + 12);
        uint32_t len = rd_u32le(raw + 16);
        uint32_t kind = rd_u32le(raw + 20);
        cat_entry *e;
        if (is_end_mark(raw))
            continue;
        if (raw[0] == 0 && off == 0 && len == 0)
            continue;
        if ((uint64_t)off + (uint64_t)len > (uint64_t)size)
            return -2;
        e = &c->entries[n];
        memcpy(e->name, raw, 12);
        e->name[12] = '\0';
        e->offset = off;
        e->length = len;
        e->kind = kind;
        n++;
    }
    c->data = data;
    c->size = size;
    c->count = n;
    return 0;
}

const cat_entry *cat_find(const catalog *c, const char *name)
{
    int i;
    if (c == NULL || name == NULL)
        return NULL;
    for (i = 0; i < c->count; i++)
        if (name_eq_ci(c->entries[i].name, name))
            return &c->entries[i];
    return NULL;
}

const uint8_t *cat_data(const catalog *c, const cat_entry *e)
{
    if (c == NULL || e == NULL || c->data == NULL)
        return NULL;
    if ((uint64_t)e->offset + (uint64_t)e->length > (uint64_t)c->size)
        return NULL;
    return c->data + e->offset;
}
