#include "catalog.h"

#include <string.h>

static uint32_t rd_u32le(const uint8_t *p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

static int is_link(const uint8_t *name)
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
    uint32_t seen[64];
    uint32_t nblocks = 0;
    uint32_t base = 0;
    int n = 0;

    if (c == NULL)
        return -1;
    c->data = NULL;
    c->size = 0;
    c->count = 0;
    c->blocks = 0;
    if (data == NULL || size < 24)
        return -1;
    for (;;) {
        uint32_t link_off = 0;
        int i, seen_i;

        if (nblocks >= 64)
            return -3;                          /* too many blocks */
        if ((uint64_t)base + 33ull * 24ull > (uint64_t)size)
            return -2;                          /* block does not fit in the buffer */
        for (seen_i = 0; seen_i < (int)nblocks; seen_i++)
            if (seen[seen_i] == base)
                return -3;                      /* link points back to a block already read */
        seen[nblocks++] = base;

        for (i = 0; i < 33; i++) {              /* 32 entries, then the link entry */
            const uint8_t *raw = data + (size_t)base + 24u * (uint32_t)i;
            if (is_link(raw)) {
                link_off = rd_u32le(raw + 12);
                break;
            }
            if (raw[0] == 0)
                continue;                       /* unused entry (empty name) */
            if (n >= 256)
                return -3;                      /* too many named entries */
            {
                uint32_t off = rd_u32le(raw + 12);
                uint32_t len = rd_u32le(raw + 16);
                uint32_t kind = rd_u32le(raw + 20);
                cat_entry *e;
                if ((uint64_t)off + (uint64_t)len > (uint64_t)size)
                    return -2;                  /* entry points outside the file */
                e = &c->entries[n];
                memcpy(e->name, raw, 12);
                e->name[12] = '\0';
                e->offset = off;
                e->length = len;
                e->kind = kind;
                n++;
            }
        }
        if (i == 33)
            return -3;                          /* no link entry in the block */
        if (link_off == 0)
            break;                              /* last block */
        base = link_off;
    }
    c->data = data;
    c->size = size;
    c->count = n;
    c->blocks = (int)nblocks;
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
