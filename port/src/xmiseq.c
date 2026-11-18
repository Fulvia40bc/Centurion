#include "xmiseq.h"

#include <stdlib.h>
#include <string.h>

#define XMI_MAX_LOOPS 4u
#define XMI_GUARD 1000000u /* max emissions per advance, against zero-delay loop bombs */

typedef struct { uint32_t for_idx; uint32_t count; } xmi_loop;

typedef struct { int active; uint64_t fire; } xmi_note;

struct xmi_seq {
    const xmi_song *song;
    xmi_out_fn out;
    xmi_trigger_fn trig;
    void *user;
    uint32_t index;        /* next event in song->ev */
    int64_t shift;         /* effective tick of event i = (int64_t)ev[i].tick + shift */
    uint64_t now;          /* current time in ticks */
    xmi_loop loops[XMI_MAX_LOOPS];
    uint32_t depth;
    xmi_note pend[16][128];
    uint32_t pending;
    int eot;
    int stopped;
    uint32_t loops_done;
};

static uint64_t seq_eff(const xmi_seq *s, uint32_t i)
{
    return (uint64_t)((int64_t)s->song->ev[i].tick + s->shift);
}

static void seq_emit(xmi_seq *s, const uint8_t *msg, int len)
{
    if (s->out != NULL)
        s->out(s->user, msg, len);
}

static void seq_note_off(xmi_seq *s, int ch, int key)
{
    uint8_t msg[3];

    msg[0] = (uint8_t)(0x80u | (unsigned)ch);
    msg[1] = (uint8_t)key;
    msg[2] = 0;
    seq_emit(s, msg, 3);
    s->pend[ch][key].active = 0;
    s->pending--;
}

/* handle the event at s->index, whose effective tick is t */
static void seq_event(xmi_seq *s, uint64_t t)
{
    const xmi_event *e = &s->song->ev[s->index];
    uint8_t st = e->status;
    uint8_t msg[3];

    if (st == 0xFFu) {              /* end of track (FF 2F) */
        s->eot = 1;
        s->index = s->song->count;
        return;
    }
    if ((st & 0xF0u) == 0x80u) {    /* note-off events in the list are skipped */
        s->index++;
        return;
    }
    if ((st & 0xF0u) == 0x90u) {    /* note-on: schedule its own note-off at t + dur */
        int ch = st & 0x0F;
        int key = e->d1;
        xmi_note *n = &s->pend[ch][key];
        int was = n->active;
        if (was) {                  /* re-struck while sounding: note-off first */
            msg[0] = (uint8_t)(0x80u | (unsigned)ch);
            msg[1] = (uint8_t)key;
            msg[2] = 0;
            seq_emit(s, msg, 3);
        }
        msg[0] = st;
        msg[1] = e->d1;
        msg[2] = e->d2;
        seq_emit(s, msg, 3);
        n->active = 1;
        n->fire = t + (uint64_t)e->dur;
        if (!was)
            s->pending++;
        s->index++;
        return;
    }
    if ((st & 0xF0u) == 0xB0u) {    /* controller */
        int num = e->d1;
        int val = e->d2;
        int ch = st & 0x0F;
        if (num >= 110 && num <= 120) {  /* XMIDI's own: never sent to out */
            if (num == 116) {            /* FOR: push the loop */
                if (s->depth < XMI_MAX_LOOPS) {
                    s->loops[s->depth].for_idx = s->index;
                    s->loops[s->depth].count = (uint32_t)val;
                    s->depth++;
                }
                s->index++;
            } else if (num == 117) {     /* NEXT */
                if (val >= 64) {
                    if (s->depth > 0) {
                        xmi_loop *lp = &s->loops[s->depth - 1];
                        int jump = 0;
                        if (lp->count == 0)
                            jump = 1;
                        else if (lp->count > 1) {
                            lp->count--;
                            jump = 1;
                        } else {
                            lp->count--;
                            s->depth--;
                        }
                        if (jump) {
                            /* play continues at the FOR's next event; the FOR's
                               position in time becomes now, so the body replays
                               after now with its original spacing */
                            s->shift = (int64_t)t - (int64_t)s->song->ev[lp->for_idx].tick;
                            s->index = lp->for_idx + 1;
                            s->loops_done++;
                        } else {
                            s->index++;
                        }
                    } else {
                        s->index++;
                    }
                } else {                /* value < 64: break out of the loop */
                    if (s->depth > 0)
                        s->depth--;
                    s->index++;
                }
            } else if (num == 119) {     /* callback trigger */
                if (s->trig != NULL)
                    s->trig(s->user, ch, val);
                s->index++;
            } else {                    /* 110-115, 118, 120: ignored */
                s->index++;
            }
            return;
        }
        msg[0] = st;
        msg[1] = e->d1;
        msg[2] = e->d2;
        seq_emit(s, msg, 3);
        s->index++;
        return;
    }
    msg[0] = st;                        /* other channel messages: as-is */
    msg[1] = e->d1;
    msg[2] = e->d2;
    seq_emit(s, msg, e->len);
    s->index++;
}

xmi_seq *xmi_seq_new(const xmi_song *song, xmi_out_fn out, xmi_trigger_fn trig, void *user)
{
    xmi_seq *s;

    if (song == NULL || (song->count > 0 && song->ev == NULL))
        return NULL;
    s = calloc(1, sizeof *s);
    if (s == NULL)
        return NULL;
    s->song = song;
    s->out = out;
    s->trig = trig;
    s->user = user;
    return s;
}

void xmi_seq_advance(xmi_seq *s, uint32_t ticks)
{
    uint64_t target;
    uint32_t guard = 0;

    if (s == NULL)
        return;
    target = s->now + (uint64_t)ticks;
    if (!s->stopped) {
        for (;;) {
            int oc = -1, ok = -1;
            uint64_t off_time = 0;
            int ev_due;
            uint64_t ev_time = 0;
            uint32_t i;

            if (s->pending > 0) {       /* earliest pending note-off */
                for (i = 0; i < 16 * 128; i++) {
                    xmi_note *n = &s->pend[0][0] + i;
                    if (n->active && (oc < 0 || n->fire < off_time)) {
                        oc = (int)(i / 128);
                        ok = (int)(i % 128);
                        off_time = n->fire;
                    }
                }
            }
            ev_due = (!s->eot && s->index < s->song->count && seq_eff(s, s->index) <= target);
            if (ev_due)
                ev_time = seq_eff(s, s->index);
            if (oc >= 0 && off_time <= target && (!ev_due || off_time <= ev_time)) {
                seq_note_off(s, oc, ok); /* at equal ticks the note-off goes first */
            } else if (ev_due) {
                seq_event(s, ev_time);
            } else {
                break;
            }
            if (++guard > XMI_GUARD)
                break;
        }
    }
    if (s->index >= s->song->count)
        s->eot = 1;
    s->now = target;
}

int xmi_seq_done(const xmi_seq *s)
{
    if (s == NULL)
        return 1;
    return s->stopped || (s->eot && s->pending == 0);
}

void xmi_seq_stop(xmi_seq *s)
{
    int ch, key;

    if (s == NULL)
        return;
    for (ch = 0; ch < 16; ch++)
        for (key = 0; key < 128; key++)
            if (s->pend[ch][key].active)
                seq_note_off(s, ch, key);
    s->pending = 0;
    s->stopped = 1;
}

uint32_t xmi_seq_loops(const xmi_seq *s)
{
    return s == NULL ? 0 : s->loops_done;
}

void xmi_seq_free(xmi_seq *s)
{
    free(s);
}
