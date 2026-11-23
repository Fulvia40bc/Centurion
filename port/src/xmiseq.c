#include "xmiseq.h"

#include <stdlib.h>
#include <string.h>

/* The channel-message stream follows the game's own XMIDI interpreter (the host model in port/test/fm_dro_cmp.c,
   translated from AC_MAIN 0x99F80..0x9A6A9 / 0x99610 / 0x99AA0 and proven against a DOSBox-X capture): one 120 Hz
   interval per tick; at the START of each interval the 32-slot note queue runs in slot order (note-offs), then the
   event list is parsed; a note-on takes the first free slot (a re-struck key takes a second slot and gets NO
   note-off first) and a duration of 0 turns off at the next interval; the queue full stops the sequence at once.
   Controllers: the shadow keeps the raw value of every controller; CC6 is preceded by CC100 0, CC101 0, CC38 0;
   CC7 goes out as raw * volume * 127 / 16129 clamped to 0..127; 109, 110, 111, 118, 119 are consumed, 116/117 are
   FOR/NEXT, everything else (107, 108, 112..115, 120+) is forwarded; a note-on is muted while the CC107 shadow is
   >= 64 (it still takes a queue slot). FOR takes the first free of 4 slots; NEXT with a value < 64 is ignored, else
   jumps the innermost open FOR (count 0 = forever). FF 2F: the loop count decides; a rewind restarts at the NEXT
   interval (shift = t + 1) and keeps the FOR slots; ending stops the notes and releases the pedal (CC64 0 and
   CC112 0 where the shadow is >= 64). pause/stop are that same emission. */

#define XMI_FOR_SLOTS 4u
#define XMI_QUEUE     32

struct xmi_seq {
    const xmi_song *song;
    xmi_out_fn out;
    xmi_trigger_fn trig;
    void *user;
    uint32_t index;        /* next event in song->ev */
    int64_t shift;         /* effective tick of event i = (int64_t)ev[i].tick + shift */
    uint64_t now;          /* current time in ticks */
    int for_count[XMI_FOR_SLOTS];      /* -1 = free, else the FOR's count (0 = forever) */
    uint32_t for_ptr[XMI_FOR_SLOTS];   /* the FOR event's index */
    int nq_ch[XMI_QUEUE];              /* -1 = free slot */
    int nq_key[XMI_QUEUE];
    uint64_t nq_fire[XMI_QUEUE];       /* the tick the note-off is due */
    int note_count;
    int eot;
    int stopped;
    int paused;            /* xmi_seq_pause: time does not advance, nothing is emitted */
    int seq_loops;         /* what happens at FF 2F (xmi_seq_set_loops, default 1) */
    int vol_scale;         /* xmi_seq_set_volume, 0..127, default 127; CC7 out as raw * vol_scale / 127 */
    int8_t shadow[16][128]; /* the last raw value of every controller sent, -1 = never sent */
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

static void seq_note_off_slot(xmi_seq *s, int i)
{
    uint8_t msg[3];

    msg[0] = (uint8_t)(0x80u | (unsigned)s->nq_ch[i]);
    msg[1] = (uint8_t)s->nq_key[i];
    msg[2] = 0;
    seq_emit(s, msg, 3);
    s->nq_ch[i] = -1;
    s->note_count--;
}

/* the stop/pause emission (AIL_stop_sequence 0x99AA0): note-off for every queued note in slot order, forget
   them, then CC64 0 and CC112 0 on each channel whose shadow is down */
static void seq_pause_notes(xmi_seq *s)
{
    int i, ch;

    for (i = 0; i < XMI_QUEUE; i++)
        if (s->nq_ch[i] != -1)
            seq_note_off_slot(s, i);
    s->note_count = 0;
    for (ch = 0; ch < 16; ch++) {
        if (s->shadow[ch][64] >= 64) {
            uint8_t msg[3];

            msg[0] = (uint8_t)(0xB0u | (unsigned)ch);
            msg[1] = 64;
            msg[2] = 0;
            seq_emit(s, msg, 3);
            s->shadow[ch][64] = 0;
        }
        if (s->shadow[ch][112] >= 64) {
            uint8_t msg[3];

            msg[0] = (uint8_t)(0xB0u | (unsigned)ch);
            msg[1] = 112;
            msg[2] = 0;
            seq_emit(s, msg, 3);
            s->shadow[ch][112] = 0;
        }
    }
}

/* one controller message to the driver queue (0x99610): shadow, CC6's prefix, CC7's scaling, the consumed
   controllers, the CC107 mute; FOR/NEXT are handled by the caller (they need the tick) */
static void seq_cc_out(xmi_seq *s, int ch, int num, int val)
{
    uint8_t msg[3];

    s->shadow[ch][num & 0x7F] = (int8_t)val;            /* the shadow keeps the raw value */
    if (num == 6) {                                     /* 0x99782: CC100 0, CC101 0, CC38 0, then CC6 */
        seq_cc_out(s, ch, 100, 0);
        seq_cc_out(s, ch, 101, 0);
        seq_cc_out(s, ch, 38, 0);
    } else if (num == 7) {                              /* 0x99810: raw * volume * 127 / 16129, clamped */
        long v = (long)val * s->vol_scale * 127 / 0x3F01;

        if (v > 127)
            v = 127;
        if (v < 0)
            v = 0;
        val = (int)v;
    } else if (num == 109 || num == 110 || num == 111 || num == 118 || num == 119) {
        if (num == 119 && s->trig != NULL)
            s->trig(s->user, ch, val);                  /* the Task 7 trigger; no channel message */
        return;                                         /* consumed */
    }
    msg[0] = (uint8_t)(0xB0u | (unsigned)ch);
    msg[1] = (uint8_t)num;
    msg[2] = (uint8_t)val;
    seq_emit(s, msg, 3);
}

/* handle the event at s->index, whose effective tick is t */
static void seq_event(xmi_seq *s, uint64_t t)
{
    const xmi_event *e = &s->song->ev[s->index];
    uint8_t st = e->status;

    if (st == 0xFFu) {              /* end of track (FF 2F): the loop count decides */
        if (s->seq_loops == 0 || --s->seq_loops != 0) {
            /* rewind: the host stops parsing for the rest of this tick (end_hit) and restarts at the next
               interval, so the shift is t + 1 and the FOR slots are kept */
            s->shift = (int64_t)t + 1;
            s->index = 0;
            s->eot = 0;
            s->loops_done++;
        } else {
            seq_pause_notes(s);     /* pause its notes and done (the count is now 0: a later rewind without a
                                       new count loops forever) */
            s->eot = 1;
            s->index = s->song->count;
        }
        return;
    }
    if ((st & 0xF0u) == 0x80u) {    /* note-off events in the list are the parser's own (xmi.c adds them at
                                       tick + dur); the queue sends the real note-offs */
        s->index++;
        return;
    }
    if ((st & 0xF0u) == 0x90u) {    /* note-on: the message (muted while CC107 >= 64), then the first free slot */
        int ch = st & 0x0F;
        int key = e->d1;
        int i;

        if (s->shadow[ch][107] < 64) {                  /* 0x99A2C: mute */
            uint8_t msg[3];

            msg[0] = st;
            msg[1] = e->d1;
            msg[2] = e->d2;
            seq_emit(s, msg, 3);
        }
        for (i = 0; i < XMI_QUEUE && s->nq_ch[i] != -1; i++)
            ;
        if (i == XMI_QUEUE) {                           /* 0x9A48E: "Internal note queue overflow" */
            seq_pause_notes(s);
            s->eot = 1;
            s->index = s->song->count;
            return;
        }
        s->nq_ch[i] = ch;                               /* no note-off first on a re-strike: a second slot */
        s->nq_key[i] = key;
        s->nq_fire[i] = t + (e->dur != 0 ? e->dur : 1); /* duration 0 turns off at the next interval */
        s->note_count++;
        s->index++;
        return;
    }
    if ((st & 0xF0u) == 0xB0u) {    /* controller */
        int num = e->d1;
        int val = e->d2;
        int ch = st & 0x0F;

        if (num == 116) {                               /* FOR: the first free of 4 slots */
            int i;

            for (i = 0; i < (int)XMI_FOR_SLOTS && s->for_count[i] != -1; i++)
                ;
            if (i < (int)XMI_FOR_SLOTS) {
                s->for_count[i] = val;
                s->for_ptr[i] = s->index;
            }
            s->index++;
            return;
        }
        if (num == 117) {                               /* NEXT: value < 64 ignored, else the innermost open FOR */
            if (val >= 64) {
                int i;

                for (i = (int)XMI_FOR_SLOTS - 1; i >= 0 && s->for_count[i] == -1; i--)
                    ;
                if (i >= 0) {
                    if (s->for_count[i] == 0 || --s->for_count[i] != 0) {
                        /* the host lands past the FOR event and keeps parsing in the same interval */
                        s->shift = (int64_t)t - (int64_t)s->song->ev[s->for_ptr[i]].tick;
                        s->index = s->for_ptr[i] + 1;
                        s->loops_done++;
                        return;
                    }
                    s->for_count[i] = -1;
                }
            }
            s->index++;
            return;
        }
        seq_cc_out(s, ch, num, val);                    /* sets the raw shadow (0x99676), scales, emits */
        s->index++;
        return;
    }
    {                                           /* other channel messages: as-is */
        uint8_t msg[3];

        msg[0] = st;
        msg[1] = e->d1;
        msg[2] = e->d2;
        seq_emit(s, msg, e->len);
    }
    s->index++;
}

/* one 120 Hz interval at s->now: the note queue first (slot order), then the events due */
static void seq_tick(xmi_seq *s)
{
    int i;

    if (s->note_count > 0) {
        for (i = 0; i < (int)XMI_QUEUE; i++) {
            if (s->nq_ch[i] == -1)
                continue;
            if (s->nq_fire[i] <= s->now)
                seq_note_off_slot(s, i);
            if (s->note_count == 0)
                break;
        }
    }
    while (!s->eot && s->index < s->song->count && seq_eff(s, s->index) <= s->now)
        seq_event(s, seq_eff(s, s->index));
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
    s->seq_loops = 1;
    s->vol_scale = 127;
    memset(s->shadow, 0xFF, sizeof s->shadow);   /* -1 = never sent */
    {
        int i;

        for (i = 0; i < (int)XMI_FOR_SLOTS; i++)
            s->for_count[i] = -1;
        for (i = 0; i < (int)XMI_QUEUE; i++)
            s->nq_ch[i] = -1;
    }
    return s;
}

void xmi_seq_advance(xmi_seq *s, uint32_t ticks)
{
    uint64_t target;

    if (s == NULL || s->paused)
        return;                                 /* paused: time does not advance, nothing is emitted */
    if (s->stopped) {
        s->now += ticks;
        return;
    }
    if (ticks == 0) {                           /* the initial flush at tick 0 (mus_start) */
        seq_tick(s);
        if (s->index >= s->song->count)
            s->eot = 1;
        return;
    }
    target = s->now + (uint64_t)ticks;
    while (s->now < target) {
        s->now++;
        if (!s->stopped)
            seq_tick(s);
    }
    s->now = target;
    if (s->index >= s->song->count)
        s->eot = 1;
}

int xmi_seq_done(const xmi_seq *s)
{
    if (s == NULL)
        return 1;
    return s->stopped || (s->eot && s->note_count == 0);
}

void xmi_seq_stop(xmi_seq *s)
{
    if (s == NULL)
        return;
    seq_pause_notes(s);                     /* AIL_end_sequence runs stop_sequence: note-offs, then CC64 0 /
                                               CC112 0 where the shadow is down (the pedal is released) */
    s->stopped = 1;
}

void xmi_seq_pause(xmi_seq *s)
{
    if (s == NULL)
        return;
    if (!s->paused && !s->stopped)
        seq_pause_notes(s);
    s->paused = 1;
}

void xmi_seq_resume(xmi_seq *s)
{
    if (s == NULL)
        return;
    s->paused = 0;                              /* from the same event and the same remaining delay */
}

void xmi_seq_rewind(xmi_seq *s)
{
    int i;

    if (s == NULL)
        return;
    if (!s->paused && !s->stopped)
        seq_pause_notes(s);                     /* pause if playing */
    s->index = 0;
    s->shift = 0;
    s->now = 0;                                 /* back to tick 0 */
    s->eot = 0;
    s->stopped = 0;
    s->paused = 1;
    for (i = 0; i < (int)XMI_FOR_SLOTS; i++)    /* FOR slots cleared */
        s->for_count[i] = -1;
    for (i = 0; i < (int)XMI_QUEUE; i++)        /* note queue empty */
        s->nq_ch[i] = -1;
    s->note_count = 0;
    memset(s->shadow, 0xFF, sizeof s->shadow);  /* shadows cleared to -1; loop count and volume are kept */
}

void xmi_seq_set_loops(xmi_seq *s, int n)
{
    if (s == NULL)
        return;
    s->seq_loops = n;
}

void xmi_seq_set_volume(xmi_seq *s, int v)
{
    int ch;

    if (s == NULL)
        return;
    if (v < 0)
        v = 0;
    if (v > 127)
        v = 127;
    if (v == s->vol_scale)
        return;
    s->vol_scale = v;
    for (ch = 0; ch < 16; ch++) {               /* resend the scaled CC7 where the song set one */
        if (s->shadow[ch][7] >= 0) {
            uint8_t msg[3];
            long x = (long)s->shadow[ch][7] * v * 127 / 0x3F01;

            if (x > 127)
                x = 127;
            if (x < 0)
                x = 0;
            msg[0] = (uint8_t)(0xB0u | (unsigned)ch);
            msg[1] = 7;
            msg[2] = (uint8_t)x;
            seq_emit(s, msg, 3);
        }
    }
}

uint32_t xmi_seq_loops(const xmi_seq *s)
{
    return s == NULL ? 0 : s->loops_done;
}

void xmi_seq_free(xmi_seq *s)
{
    free(s);
}
