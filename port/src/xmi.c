#include "xmi.h"

#include <stdlib.h>
#include <string.h>

static uint32_t rd_u32be(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24)
         | ((uint32_t)p[1] << 16)
         | ((uint32_t)p[2] << 8)
         | (uint32_t)p[3];
}

static uint16_t rd_u16le(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

typedef struct {
    xmi_event *ev;
    uint32_t count;
    uint32_t cap;
} ev_list;

static int ev_push(ev_list *l, uint32_t tick, uint8_t status, uint8_t d1, uint8_t d2, uint8_t len)
{
    if (l->count == l->cap) {
        uint32_t ncap = l->cap == 0 ? 256 : l->cap * 2;
        xmi_event *nev = realloc(l->ev, (size_t)ncap * sizeof *nev);
        if (nev == NULL)
            return -5;
        l->ev = nev;
        l->cap = ncap;
    }
    l->ev[l->count].tick = tick;
    l->ev[l->count].status = status;
    l->ev[l->count].d1 = d1;
    l->ev[l->count].d2 = d2;
    l->ev[l->count].len = len;
    l->count++;
    return 0;
}

static int ev_key(const xmi_event *e)
{
    return ((e->status & 0xF0u) == 0x80u) ? 0 : 1;
}

static int ev_less(const xmi_event *a, const xmi_event *b)
{
    if (a->tick != b->tick)
        return a->tick < b->tick;
    return ev_key(a) < ev_key(b);
}

static void merge_sort_ev(xmi_event *ev, xmi_event *tmp, uint32_t lo, uint32_t hi)
{
    uint32_t mid, i, j, k;

    if (hi - lo < 2)
        return;
    mid = lo + (hi - lo) / 2;
    merge_sort_ev(ev, tmp, lo, mid);
    merge_sort_ev(ev, tmp, mid, hi);
    i = lo;
    j = mid;
    k = lo;
    while (i < mid && j < hi) {
        if (ev_less(&ev[j], &ev[i]))
            tmp[k++] = ev[j++];
        else
            tmp[k++] = ev[i++];
    }
    while (i < mid)
        tmp[k++] = ev[i++];
    while (j < hi)
        tmp[k++] = ev[j++];
    memcpy(ev + lo, tmp + lo, (size_t)(hi - lo) * sizeof *ev);
}

static int read_vlq(const uint8_t *data, size_t end, size_t *pos, uint32_t *out)
{
    uint32_t v = 0;
    uint8_t b;

    do {
        if (*pos >= end)
            return -2;
        b = data[(*pos)++];
        v = (v << 7) | (uint32_t)(b & 0x7Fu);
    } while (b & 0x80u);
    *out = v;
    return 0;
}

static int parse_evnt(const uint8_t *data, size_t start, size_t end, ev_list *l)
{
    size_t pos = start;
    uint32_t tick = 0;

    while (pos < end) {
        uint8_t status = data[pos++];
        if (status < 0x80u) {
            tick += status;
            continue;
        }
        if (status < 0xF0u) {
            unsigned type = status & 0xF0u;
            unsigned ndata = (type == 0xC0u || type == 0xD0u) ? 1u : 2u;
            uint8_t d1, d2 = 0;
            int rc;
            if (pos + ndata > end)
                return -2;
            d1 = data[pos++];
            if (ndata == 2)
                d2 = data[pos++];
            if (type == 0x90u) {
                uint32_t dur;
                rc = read_vlq(data, end, &pos, &dur);
                if (rc != 0)
                    return rc;
                rc = ev_push(l, tick, status, d1, d2, 3);
                if (rc != 0)
                    return rc;
                rc = ev_push(l, tick + dur, (uint8_t)(0x80u | (status & 0x0Fu)), d1, 0, 3);
                if (rc != 0)
                    return rc;
            } else {
                rc = ev_push(l, tick, status, d1, d2, (uint8_t)(ndata + 1));
                if (rc != 0)
                    return rc;
            }
        } else if (status == 0xF0u || status == 0xF7u) {
            uint32_t slen;
            int rc = read_vlq(data, end, &pos, &slen);
            if (rc != 0)
                return rc;
            if (slen > end - pos)
                return -2;
            pos += slen;
        } else if (status == 0xFFu) {
            uint8_t mtype;
            uint32_t mlen;
            int rc;
            if (pos >= end)
                return -2;
            mtype = data[pos++];
            rc = read_vlq(data, end, &pos, &mlen);
            if (rc != 0)
                return rc;
            if (mtype == 0x2Fu) {
                rc = ev_push(l, tick, 0xFF, 0x2F, 0x00, 3);
                if (rc != 0)
                    return rc;
                return 0;
            }
            if (mlen > end - pos)
                return -2;
            pos += mlen;
        } else {
            return -4;
        }
    }
    return 0;
}

typedef struct {
    const uint8_t *data;
    size_t len;
    int in_xmid;
    int done;
    ev_list ev;
    uint8_t timb[64][2];
    int ntimb;
    int have_timb;
} xmi_walk;

static int walk_chunks(xmi_walk *w, size_t start, size_t end);

static int walk_one(xmi_walk *w, size_t pos, size_t end)
{
    size_t size;
    const uint8_t *tag;

    if (pos + 8 > end)
        return -2;
    tag = w->data + pos;
    size = rd_u32be(w->data + pos + 4);
    if (size > end - pos - 8)
        size = end - pos - 8;
    if (memcmp(tag, "FORM", 4) == 0 || memcmp(tag, "CAT ", 4) == 0 || memcmp(tag, "LIST", 4) == 0) {
        int rc;
        if (size < 4)
            return -3;
        if (memcmp(w->data + pos + 8, "XMID", 4) == 0) {
            w->in_xmid = 1;
            rc = walk_chunks(w, pos + 12, pos + 8 + size);
            w->in_xmid = 0;
        } else {
            rc = walk_chunks(w, pos + 12, pos + 8 + size);
        }
        if (rc != 0)
            return rc;
    } else if (w->in_xmid && memcmp(tag, "EVNT", 4) == 0 && !w->done) {
        int rc = parse_evnt(w->data, pos + 8, pos + 8 + size, &w->ev);
        if (rc != 0)
            return rc;
        w->done = 1;
    } else if (w->in_xmid && memcmp(tag, "TIMB", 4) == 0 && !w->have_timb) {
        size_t p = pos + 8;
        size_t avail = size;
        unsigned n, i;
        if (avail < 2)
            return -3;
        n = rd_u16le(w->data + p);
        p += 2;
        avail -= 2;
        if ((size_t)n * 2 > avail)
            n = (unsigned)(avail / 2);
        if (n > 64)
            n = 64;
        for (i = 0; i < n; i++) {
            w->timb[i][0] = w->data[p + 2 * i];
            w->timb[i][1] = w->data[p + 2 * i + 1];
        }
        w->ntimb = (int)n;
        w->have_timb = 1;
    }
    return 0;
}

static int walk_chunks(xmi_walk *w, size_t start, size_t end)
{
    size_t pos = start;

    while (pos + 8 <= end) {
        size_t size = rd_u32be(w->data + pos + 4);
        int rc = walk_one(w, pos, end);
        if (rc != 0)
            return rc;
        if (size > end - pos - 8)
            size = end - pos - 8;
        pos += 8 + size + (size & 1u);
    }
    return 0;
}

int xmi_parse(const uint8_t *data, size_t len, xmi_song *out)
{
    xmi_walk w;
    int rc;

    if (out == NULL)
        return -1;
    memset(out, 0, sizeof *out);
    if (data == NULL || len < 8)
        return -1;
    if (memcmp(data, "FORM", 4) != 0)
        return -3;
    memset(&w, 0, sizeof w);
    w.data = data;
    w.len = len;
    rc = walk_chunks(&w, 0, len);
    if (rc != 0) {
        free(w.ev.ev);
        return rc;
    }
    if (!w.done) {
        free(w.ev.ev);
        return -3;
    }
    if (w.ev.count > 1) {
        xmi_event *tmp = malloc((size_t)w.ev.count * sizeof *tmp);
        if (tmp == NULL) {
            free(w.ev.ev);
            return -5;
        }
        merge_sort_ev(w.ev.ev, tmp, 0, w.ev.count);
        free(tmp);
    }
    out->ev = w.ev.ev;
    out->count = w.ev.count;
    memcpy(out->timb, w.timb, sizeof w.timb);
    out->ntimb = w.ntimb;
    return 0;
}

void xmi_free(xmi_song *s)
{
    if (s == NULL)
        return;
    free(s->ev);
    s->ev = NULL;
    s->count = 0;
    s->ntimb = 0;
}
