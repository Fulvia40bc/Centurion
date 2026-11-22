/* Module MOVE of docs/MAP_PLAN.md: the movement primitives 0x57CE8, 0x57E28, 0x580CC, 0x581C4, 0x576F8, 0x576A8,
   0x57A10, 0x57B5C and the movement cases of the button dispatcher 0x3FB94 (ids 0x26..0x29, 0x2B..0x32, 0xC4..0xC8),
   as dcase_<id>(code) for dispatch.c. Literal translations; the address of each instruction a line comes from is in
   its comment. D = read in the code, G = inference. Prototypes and the record layout: move.h.

   Who calls the cases (D): the movement phase 0x55574 (not ported) waits for a click in its own loop (0x55FFD: fg_mousepos
   until [0xE1630] != 0) and hands it to the map click 0x3D434, which runs f3C5D4 and f3FB94(id) for a panel button and
   returns its code, or the territory for a map click. The phase itself moves tokens (0x564E9 f57CE8, 0x564FD f57E28)
   and ships (0x56B44 f580CC) on the second territory click and records the move for UNDO MOVE:
     int32 [0x108810]  1 = the last move was a land move, else a ship move
     int32 [0x108808]  its source territory;  int32 [0x10880C]  its destination
     int32 [0x108814]  1 = it was the stack's first step (f57CE8 then also took the tokens off +0x24 of the source)
     int32 [0x108818]  the token count (int16 [0xF09E0] at the time)
     int32 [0x10881C]  steps of the current stack (0 when a source is chosen, + 1 per land step, - 1 by UNDO)
   Other data: int32 [0x109E10] the territory the panel shows; int32 [0xF06CC] 1 = a source is chosen; int8 [0x108804]
   1 land mode, 2 sea mode; int16 [0xF09E0] the count chosen with 0xC4..0xC8; int32 [0xF40F8] the moving nation;
   0xC803C uint8[160] the canonical territory (mapdata.h).
   The play-by-email move log (D): uint32 [0xF09B4] a 10000-byte heap block (malloc at 0x55E5A, only when bit 0x80 of
   [0xE1624] is set; realloc'd at 0x56090 / 0x57045 and kept in 0xF0984[n]), int16 [0xF09B8] its record count;
   10-byte records, +0 / +2 territory, +8 the action (3 EMBARK, 4 DISEMBARK, 5 DISEMBARK ALL, 7..9 the second
   coast's, 0x0B / 0x0C NEXT SHIP), +9 the token count. The field holds a port handle (load_heap_ptr; PBEM is dropped
   by Erik's ruling, so the port never allocates it: the writes below go through a guard that skips them when the
   handle does not resolve).

   Small helpers the cases need, all ported elsewhere: f3B9A4, f3B7CC, f3B870 (game.h), f3BFBC, f3C3D0 (ui.h),
   f3CBBC, f3D52C (map.h), f3E89C, f3E8FC, f3F698, f3F70C, f3F670, f3F624, f3DA60 (mapdata.h), f426D0, f572CC
   (shippanel.h), fg_vbopen / fg_vbclose (fg.h). No unported callee: nothing here calls not_ported.
   Runtime: __CHK dropped. Byte arithmetic on the records wraps as on the 386 (uint8_t stores).

   1995 bug fixed (the original's behaviour kept under g_original, marked BUG(1995)):
     1. NEXT SHIP, both buttons (0x2E at 0x41805..0x41850, the second coast's 0x32 at 0x41730..0x4177A), writes its
        play-by-email log record and bumps int16 [0xF09B8] without the test of bit 0x80 of [0xE1624] that the other
        six ship actions and UNDO make (0x40EF1, 0x4101F, 0x41193, 0x412F1, 0x4149A, 0x41633, 0x40DF9). Outside PBEM
        [0xF09B4] is 0 (D: its only writers 0x55E61, 0x56095 and 0x5704A run only with bit 0x80, behind the tests at
        0x55E2A, 0x5606E and 0x5701F, and the data object starts with 0 there; the phase's own log writes are
        guarded too, 0x5681E and 0x56DC3), so each press writes two bytes at linear address 10 * count and
        10 * count + 8 under DOS/4GW's flat selector, and the count grows over the whole session: the real-mode
        interrupt vectors from the first press (byte 0x26, INT 9's segment, on the fourth), then the BIOS data area
        and DOS memory (G for the effect).
        The fix makes the log conditional, as everywhere else. In the port the original's writes cannot reach low
        memory (the guard skips them); --original keeps the counter's growth, which a data-object comparison sees
        at 0xF09B8.

   Oddities kept (D unless said):
     - f576F8, f57A10 and f57B5C compare the whole int16 ship word with t + 1; the panel, 0x57810, 0x57BD8 and these
       cases use only its low byte; f580CC keeps the high byte. Every writer found stores territory + 1 with a zero high
       byte except 0x36F18 (bx after a byte load and inc ebx, G), so they agree in practice.
     - f576A8 calls f57B5C and compares the result with 1, then returns 5 either way (no branch on the compare).
     - f581C4 goes on with the next index after removing an entry, so a second copy of t + 1 right after the first
       would stay; f57E28 never appends a copy.
     - f581C4 drops nation n from a territory where it has ships but no tokens (the markers count tokens only).
     - DSMBRK / DSMBRK-A of the second coast (0x30, 0x31) take the destination territory from the ship selected in the
       record of [0x109E10] but unload the ship selected in the record of f3E89C([0x109E10]); 0x30 also refreshes the
       first one's territory. Both ships stand in the same canonical territory whenever f576F8 has run for both records
       (the phase does so on every choice), so the tokens land where they should (G). The group-1 buttons acting on
       the twin record when [0x109E10] is itself a twin id is shippanel.c's note.
     - EMBARK / DISEMBARK do not check the load or the tokens left; the panel enables them only when they apply
       (f5D14C). DISEMBARK ALL loops while the load is non-zero, so a negative load would unload until it wraps (G:
       never happens). */

#include <stdint.h>
#include "state.h"
#include "game.h"
#include "fg.h"
#include "map.h"
#include "mapdata.h"
#include "shippanel.h"
#include "move.h"

#define MV_DLG 0xF596Cu   /* the map screen's panel dialog (D: every case passes the immediate 0xF596C) */

/* area record of territory t (record t + 1), field off + n: 69(t + 1) + n + 0xE31B8 + off, 32-bit wrap as the code */
static uint32_t ar(int32_t t, int32_t n, uint32_t off)
{
    return GAME_AREA + 69u * (uint32_t)(t + 1) + (uint32_t)n + off;
}

/* nation record n: 0xF14CC + 199n (imul reg, n, 0xC7) */
static uint32_t nrec(int32_t n)
{
    return GAME_NATION + 199u * (uint32_t)n;
}

/* low byte of ship k's word (mov ax, word [rec + 2k + 0x45]; xor ah, ah; cwde / movsx: 0..255) */
static int32_t ship_lo(int32_t n, int32_t k)
{
    return (int32_t)D8(nrec(n) + 2u * (uint32_t)k + 0x45u);
}

/* one byte of the PBEM log at the link address a = [0xF09B4] + offset: a port handle plus the offset. Port guard: the
   original writes through whatever the field holds; the port skips a handle that does not resolve (0 outside PBEM). */
static void pbem_put(uint32_t a, uint8_t v)
{
    uint8_t *p = load_heap_ptr(a, 1);
    if (p != NULL)
        *p = v;
}

/* the log record of EMBARK / DISEMBARK and the second coast's (0x40F15..0x40F58 and its copies): +2 = byte [0x109E10],
   +8 = kind, +9 = count, then int16 [0xF09B8] + 1. Each store reads the index again (dword [0xF09B6] sar 16). */
static void pbem_log(uint8_t kind, uint8_t count)
{
    pbem_put(D32(0xF09B4u) + 10u * (uint32_t)(int32_t)S16(0xF09B8u) + 2u, D8(0x109E10u));   /* mov [edx+2], al */
    pbem_put(D32(0xF09B4u) + 10u * (uint32_t)(int32_t)S16(0xF09B8u) + 8u, kind);   /* mov [edx+2*eax+8] */
    pbem_put(D32(0xF09B4u) + 10u * (uint32_t)(int32_t)S16(0xF09B8u) + 9u, count);  /* mov [edx+..+9] */
    setD16(0xF09B8u, (uint16_t)(D16(0xF09B8u) + 1u));                              /* inc word [0xF09B8] */
}

/* NEXT SHIP's log record (0x41730..0x4177A / 0x41805..0x41850): +0 = byte [0x109E10]; the index read again as a word,
   the count stored + 1, then +8 = kind at the index read before the store */
static void pbem_log_next(uint8_t kind)
{
    int32_t idx;

    pbem_put(D32(0xF09B4u) + 10u * (uint32_t)(int32_t)S16(0xF09B8u), D8(0x109E10u));      /* mov [eax], dl */
    idx = (int32_t)S16(0xF09B8u);                                    /* mov ax, word [0xF09B8]; movsx edx, ax */
    setD16(0xF09B8u, (uint16_t)(D16(0xF09B8u) + 1u));                /* mov bx, word; inc ebx; mov word [0xF09B8], bx */
    pbem_put(D32(0xF09B4u) + 10u * (uint32_t)idx + 8u, kind);        /* mov byte [edx + 2*eax + 8], kind */
}

/* ==== the primitives ============================================================================================ */

/* 0x57CE8: k tokens of nation n off territory t (eax t, edx n, ebx flag, ecx k) */
void f57CE8(int t, int n, int flag, int k)
{
    uint32_t a;

    if (k <= 0)                                                     /* 0x57D04 cmp [ebp-4], 0; jle 0x57D5A */
        return;
    a = ar(t, n, 0x0Eu);                                            /* 0x57D0A..0x57D1D 69(t+1) + n */
    setD8(a, (uint8_t)(D8(a) - (uint8_t)k));                        /* 0x57D22 sub byte [.. + 0xE31C6], al */
    if (flag == 1) {                                                /* 0x57D2B cmp ebx, 1; jne 0x57D4F */
        a = GAME_AREA + 69u * (uint32_t)(t + flag) + (uint32_t)n + 0x24u;   /* 0x57D33 add edx, ebx: t + 1 */
        setD8(a, (uint8_t)(D8(a) - (uint8_t)k));                    /* 0x57D49 sub byte [.. + 0xE31DC], al */
    }
    f581C4(t, n);                                                   /* 0x57D55 */
}

/* 0x57E28: k tokens of nation n onto territory t (eax t, edx n, ebx k) */
void f57E28(int t, int n, int k)
{
    uint32_t rec;                                                   /* [ebp-0xC] */
    int32_t fresh;                                                  /* [ebp-4] */
    int32_t i;                                                      /* [ebp-8] */
    int32_t idx;
    uint32_t a;

    if (k <= 0)                                                     /* 0x57E43 cmp [ebp-0x10], 0; jle 0x57F4C */
        return;
    rec = nrec(n);                                                  /* 0x57E4D..0x57E5B */
    if ((int32_t)S8(ar(t, n, 0x01u)) != n) {                        /* 0x57E71 dword [.. + 0xE31B6] sar 24; cmp esi */
        a = GAME_AREA + 69u * (uint32_t)(t + 1) + 0x01u;            /* 0x57E8C [edx + 4*eax + 0xE31B9]: +1 */
        setD8(a, (uint8_t)(D8(a) + 1u));                            /* inc byte */
        setD8(ar(t, n, 0x01u), (uint8_t)n);                         /* 0x57EAB mov [.. + 0xE31B9], al (n) */
    }
    a = ar(t, n, 0x0Eu);
    setD8(a, (uint8_t)(D8(a) + (uint8_t)k));                        /* 0x57EC9 mov bh,[..]; add bh,al; 0x57ED6 mov */
    fresh = 1;                                                      /* 0x57ED1 mov edi, 1; 0x57EDC */
    for (i = 0; (int32_t)S8(rec + 0x60u) > i; i++) {                /* 0x57EEF dword [rec + 0x5D] sar 24; jle */
        if ((int32_t)D8(rec + (uint32_t)i + 0x61u) == t + 1) {      /* 0x57F00 byte [rec + i + 0x61]; 0x57F0D cmp */
            fresh = 0;                                              /* 0x57F11 */
            break;                                                  /* falls into 0x57F16 */
        }
    }
    if (fresh != 0) {                                               /* 0x57F16 cmp [ebp-4], 0; je 0x57F4C */
        idx = (int32_t)S8(rec + 0x60u);                             /* 0x57F28 dword [199n + 0xF1529] sar 24 */
        setD8(rec + (uint32_t)idx + 0x61u, (uint8_t)(t + 1));       /* 0x57F38 mov [edx + eax + 0xF152D], bl */
        setD8(rec + 0x60u, (uint8_t)(D8(rec + 0x60u) + 1u));        /* 0x57F46 inc byte [199n + 0xF152C] */
    }
}

/* 0x580CC: nation n's selected ship of territory a to territory b (eax a, edx b, ebx n) */
void f580CC(int a, int b, int n)
{
    uint32_t rec = nrec(n);                                         /* 0x580E5..0x580F3 [ebp-4] */
    uint32_t p;
    uint16_t w;

    p = ar(a, n, 0x1Bu);
    setD8(p, (uint8_t)(D8(p) - 1u));                                /* 0x58109 dec byte [.. + 0xE31D3] */
    f581C4(a, n);                                                   /* 0x58115 */
    /* 0x5812D..0x58147: ax = word [rec + 2k + 0x45], xor al, al; ebx = b + 1 | eax; only bx is stored */
    w = (uint16_t)((D16(rec + 2u * (uint32_t)D8(ar(a, n, 0x2Du)) + 0x45u) & 0xFF00u) | (uint16_t)(b + 1));
    setD16(rec + 2u * (uint32_t)D8(ar(a, n, 0x2Du)) + 0x45u, w);    /* 0x5815C k read again; 0x5816C mov [eax+0x45], bx */
    setD8(ar(b, n, 0x2Du), D8(ar(a, n, 0x2Du)));                    /* 0x58198 / 0x5819E selection of b = that of a */
    p = ar(b, n, 0x1Bu);
    setD8(p, (uint8_t)(D8(p) + 1u));                                /* 0x581B7 inc byte [.. + 0xE31D3] */
}

/* 0x581C4: nation n leaves territory t when it has no tokens there (eax t, edx n) */
void f581C4(int t, int n)
{
    uint32_t rec;                                                   /* [ebp-4] */
    int32_t j;                                                      /* [ebp-0xC] */
    int32_t i;                                                      /* [ebp-8] */
    uint32_t a;

    if (D8(ar(t, n, 0x01u)) == 0)                                   /* 0x581EF cmp byte [.. + 0xE31B9], 0; je */
        return;
    if ((int32_t)S8(ar(t, n, 0x0Eu)) != 0)                          /* 0x5820F dword [.. + 0xE31C3] sar 24; jne */
        return;
    rec = nrec(n);                                                  /* 0x58220..0x5822E */
    a = GAME_AREA + 69u * (uint32_t)(t + 1) + 0x01u;                /* 0x5823C [edx + 4*eax + 0xE31B9]: +1 */
    setD8(a, (uint8_t)(D8(a) - 1u));                                /* dec byte */
    setD8(ar(t, n, 0x01u), 0);                                      /* 0x5825A mov [.. + 0xE31B9], bh (0) */
    for (j = 0; (int32_t)S8(rec + 0x60u) > j; j++) {                /* 0x5829A dword [rec + 0x5D] sar 24; 0x582A6 jle */
        if ((int32_t)D8(rec + (uint32_t)j + 0x61u) != t + 1)        /* 0x582AE; 0x582BA cmp; jne 0x58294 */
            continue;
        for (i = j; (int32_t)S8(rec + 0x60u) - 1 > i; i++)          /* 0x582C1; 0x5826B sar 24; dec eax; cmp; jle */
            setD8(rec + (uint32_t)i + 0x61u, D8(rec + (uint32_t)i + 0x62u));   /* 0x58286 / 0x58289 */
        setD8(rec + 0x60u, (uint8_t)(D8(rec + 0x60u) - 1u));        /* 0x58291 dec byte [rec + 0x60]; then j++ */
    }
}

/* 0x576F8: the current nation's selection in territory t onto one of its ships there (eax t) */
void f576F8(int t)
{
    int32_t d = 0;                                                  /* [ebp-4] = 0 (0x57710) */
    int32_t n;
    uint32_t a;
    uint8_t v;

    do {
        n = S32(0xF40F8u);                                          /* 0x57721 mov ebx, [0xF40F8] */
        a = ar(t, n, 0x2Du);                                        /* 0x57729 [ebx + eax + 0xE31E5] */
        /* 0x5773B dword [199n + 2k + 0xF150F] sar 16: the whole int16 word +0x45 + 2k; dec; cmp [ebp-8] */
        if ((int32_t)S16(nrec(n) + 2u * (uint32_t)D8(a) + 0x45u) - 1 == t)
            return;                                                 /* 0x57749 je 0x577A6 */
        v = (uint8_t)(D8(a) + 1u);                                  /* 0x5775B mov dl, [..]; inc dl */
        setD8(a, v);                                                /* 0x57764 */
        setD8(a, (uint8_t)((int32_t)v % 4));                        /* 0x5777B idiv ecx (4); 0x5778F mov [..], cl */
        d++;                                                        /* 0x57799 inc dword [ebp-4] */
    } while (d <= 4);                                               /* 0x5779C cmp [ebp-4], 4; jle 0x57713 */
}

/* 0x576A8: x if nation n has a ship with movement points left at t, else 5 (eax t, edx n, ebx x) */
int f576A8(int t, int n, int x)
{
    int32_t r;                                                      /* [ebp-4] */

    if (f57A10(t, n) == 1) {                                        /* 0x576C7; 0x576CC cmp eax, 1; jne 0x576D9 */
        r = x;                                                      /* 0x576D4 */
    } else {
        (void)f57B5C(t, n);                                         /* 0x576DF; 0x576E4 cmp eax, 1: no branch follows */
        r = 5;                                                      /* 0x576E7 */
    }
    return r;                                                       /* 0x576EE */
}

/* 0x57A10: 1 if nation n has a ship at t with movement points left (eax t, edx n) */
int f57A10(int t, int n)
{
    uint32_t rec = nrec(n);                                         /* 0x57A27..0x57A35 [ebp-4] */
    int32_t k;                                                      /* [ebp-8] */

    for (k = 0; (int32_t)D8(rec + 0x40u) > k; k++) {                /* 0x57A48 byte [rec + 0x40] & 0xFF; jle 0x57A87 */
        if ((int32_t)S16(rec + 2u * (uint32_t)k + 0x45u) != t + 1)  /* 0x57A5D dword [rec + 2k + 0x43] sar 16; jne */
            continue;
        if ((int32_t)D8(rec + (uint32_t)k + 0x41u) > 0)             /* 0x57A71 byte & 0xFF; test; jle */
            return 1;                                               /* 0x57A7D */
    }
    return 0;                                                       /* 0x57A87 */
}

/* 0x57B5C: 1 if nation n has a ship at t with tokens aboard (eax t, edx n) */
int f57B5C(int t, int n)
{
    uint32_t rec = nrec(n);                                         /* 0x57B73..0x57B81 [ebp-4] */
    int32_t k;                                                      /* [ebp-8] */

    for (k = 0; (int32_t)D8(rec + 0x40u) > k; k++) {                /* 0x57B94; jle 0x57BCE */
        if (D8(rec + (uint32_t)k + 0x4Du) == 0)                     /* 0x57BA7 mov dl, [eax + 0x4D]; test dl, dl; je */
            continue;
        if ((int32_t)S16(rec + 2u * (uint32_t)k + 0x45u) == t + 1)  /* 0x57BB6 dword [rec + 2k + 0x43] sar 16; jne */
            return 1;                                               /* 0x57BC4 */
    }
    return 0;                                                       /* 0x57BCE */
}

/* ==== the dispatcher cases ===================================================================================== */

/* id 0x26 -> 0x407DC MOVE TOKENS (land mode) */
int dcase_26(int code)
{
    if (f3B9A4(MV_DLG, 0x26) != 0)                                  /* 0x407E6; 0x407ED jne 0x426C1 */
        return code;
    fg_vbopen(S32(0xF52C8u));                                       /* 0x407FA */
    f3B7CC(MV_DLG, 0x26);                                           /* 0x4080C pressed */
    setD8(0x108804u, 1);                                            /* 0x40813 mov al, 1; 0x4081A mov [0x108804], al */
    setD32(0xF06CCu, 0);                                            /* 0x40811 xor ecx, ecx; 0x40824 mov [0xF06CC], ecx */
    f572CC(S32(0x109E10u));                                         /* 0x4081F; 0x4082A the count buttons */
    f3B870(MV_DLG, 0x27);                                           /* 0x40839 MOVE SHIPS up */
    fg_vbclose();                                                   /* 0x4083E */
    f3CBBC();                                                       /* 0x40843 */
    code = -3;                                                      /* 0x40834 mov esi, -3; 0x40848 mov [ebp-0x3C], esi */
    return code;                                                    /* 0x4084B */
}

/* ids 0xC4..0xC8: the token count 1..5 */
int dcase_C4(int code)   /* -> 0x40858 */
{
    if (f3B9A4(MV_DLG, 0xC4) != 0)                                  /* 0x40862; 0x40869 jne 0x426C1 */
        return code;
    fg_vbopen(S32(0xF52C8u));                                       /* 0x40876 */
    f3B7CC(MV_DLG, 0xC4);                                           /* 0x40888 pressed */
    f3B870(MV_DLG, 0xC5);                                           /* 0x40897 */
    f3B870(MV_DLG, 0xC6);                                           /* 0x408A6 */
    f3B870(MV_DLG, 0xC7);                                           /* 0x408B5 */
    f3B870(MV_DLG, 0xC8);                                           /* 0x408C4 */
    fg_vbclose();                                                   /* 0x408C9 */
    f3CBBC();                                                       /* 0x408CE */
    setD16(0xF09E0u, 1);                                            /* 0x408D3 mov word [0xF09E0], 1 */
    return code;                                                    /* 0x408DC mov eax, [ebp-0x3C] */
}

int dcase_C5(int code)   /* -> 0x408E9 */
{
    if (f3B9A4(MV_DLG, 0xC5) != 0)                                  /* 0x408F3; 0x408FA */
        return code;
    fg_vbopen(S32(0xF52C8u));                                       /* 0x40906 */
    f3B7CC(MV_DLG, 0xC5);                                           /* 0x40918 */
    f3B870(MV_DLG, 0xC4);                                           /* 0x40927 */
    f3B870(MV_DLG, 0xC6);                                           /* 0x40936 */
    f3B870(MV_DLG, 0xC7);                                           /* 0x40945 */
    f3B870(MV_DLG, 0xC8);                                           /* 0x40954 */
    fg_vbclose();                                                   /* 0x40959 */
    f3CBBC();                                                       /* 0x40963 */
    setD16(0xF09E0u, 2);                                            /* 0x4095E mov edx, 2; 0x40968 mov [0xF09E0], dx */
    return code;                                                    /* 0x4096F */
}

int dcase_C6(int code)   /* -> 0x4097C */
{
    if (f3B9A4(MV_DLG, 0xC6) != 0)                                  /* 0x40986; 0x4098D */
        return code;
    fg_vbopen(S32(0xF52C8u));                                       /* 0x4099A */
    f3B7CC(MV_DLG, 0xC6);                                           /* 0x409AC */
    f3B870(MV_DLG, 0xC4);                                           /* 0x409BB */
    f3B870(MV_DLG, 0xC5);                                           /* 0x409CA */
    f3B870(MV_DLG, 0xC7);                                           /* 0x409D9 */
    f3B870(MV_DLG, 0xC8);                                           /* 0x409E8 */
    fg_vbclose();                                                   /* 0x409ED */
    f3CBBC();                                                       /* 0x409F7 */
    setD16(0xF09E0u, 3);                                            /* 0x409F2 mov ebx, 3; 0x409FC mov [0xF09E0], bx */
    return code;                                                    /* 0x40A03 */
}

int dcase_C7(int code)   /* -> 0x40A10 */
{
    if (f3B9A4(MV_DLG, 0xC7) != 0)                                  /* 0x40A1A; 0x40A21 */
        return code;
    fg_vbopen(S32(0xF52C8u));                                       /* 0x40A2E */
    f3B7CC(MV_DLG, 0xC7);                                           /* 0x40A40 */
    f3B870(MV_DLG, 0xC4);                                           /* 0x40A4F */
    f3B870(MV_DLG, 0xC5);                                           /* 0x40A5E */
    f3B870(MV_DLG, 0xC6);                                           /* 0x40A6D */
    f3B870(MV_DLG, 0xC8);                                           /* 0x40A7C */
    fg_vbclose();                                                   /* 0x40A81 */
    f3CBBC();                                                       /* 0x40A8B */
    setD16(0xF09E0u, 4);                                            /* 0x40A86 mov ecx, 4; 0x40A90 mov [0xF09E0], cx */
    return code;                                                    /* 0x40A97 */
}

int dcase_C8(int code)   /* -> 0x40AA4 */
{
    if (f3B9A4(MV_DLG, 0xC8) != 0)                                  /* 0x40AAE; 0x40AB5 */
        return code;
    fg_vbopen(S32(0xF52C8u));                                       /* 0x40AC2 */
    f3B7CC(MV_DLG, 0xC8);                                           /* 0x40AD4 */
    f3B870(MV_DLG, 0xC4);                                           /* 0x40AE3 */
    f3B870(MV_DLG, 0xC5);                                           /* 0x40AF2 */
    f3B870(MV_DLG, 0xC6);                                           /* 0x40B01 */
    f3B870(MV_DLG, 0xC7);                                           /* 0x40B15 */
    fg_vbclose();                                                   /* 0x40B1A */
    f3CBBC();                                                       /* 0x40B1F */
    setD16(0xF09E0u, 5);                                            /* 0x40B10 mov esi, 5; 0x40B24 mov [0xF09E0], si */
    return code;                                                    /* 0x40B2B */
}

/* id 0x27 -> 0x40B38 MOVE SHIPS (sea mode) */
int dcase_27(int code)
{
    if (f3B9A4(MV_DLG, 0x27) != 0)                                  /* 0x40B42; 0x40B49 jne 0x426C1 */
        return code;
    fg_vbopen(S32(0xF52C8u));                                       /* 0x40B56 */
    f3B7CC(MV_DLG, 0x27);                                           /* 0x40B6A pressed */
    setD8(0x108804u, 2);                                            /* 0x40B71 mov ah, 2; 0x40B78 mov [0x108804], ah */
    setD32(0xF06CCu, 0);                                            /* 0x40B68 xor edi, edi; 0x40B83 mov [0xF06CC], edi */
    f3B870(MV_DLG, 0x26);                                           /* 0x40B89 MOVE TOKENS up */
    f3BFBC(MV_DLG, 0xC4, 0);                                        /* 0x40B98 (ebx = 0 from 0x40B6F) */
    f3BFBC(MV_DLG, 0xC5, 0);                                        /* 0x40BA9 */
    f3BFBC(MV_DLG, 0xC6, 0);                                        /* 0x40BBA */
    f3BFBC(MV_DLG, 0xC7, 0);                                        /* 0x40BCB */
    f3BFBC(MV_DLG, 0xC8, 0);                                        /* 0x40BDC */
    f3B870(MV_DLG, 0xC4);                                           /* 0x40BEB */
    f3B870(MV_DLG, 0xC5);                                           /* 0x40BFA */
    f3B870(MV_DLG, 0xC6);                                           /* 0x40C09 */
    f3B870(MV_DLG, 0xC7);                                           /* 0x40C18 */
    f3B870(MV_DLG, 0xC8);                                           /* 0x40C27 */
    fg_vbclose();                                                   /* 0x40C2C */
    f3CBBC();                                                       /* 0x40C31 */
    code = -2;                                                      /* 0x40C36 mov [ebp-0x3C], -2 */
    return code;                                                    /* 0x40C3D */
}

/* id 0x28 -> 0x40C4A UNDO MOVE: the move the phase recorded in 0x108808..0x10881C, backwards */
int dcase_28(int code)
{
    int32_t n;
    int32_t k;
    uint32_t a;
    int color;

    if (S32(0x108810u) == 1) {                                      /* 0x40C4A cmp [0x108810], 1; jne 0x40D44 */
        /* tokens: off the destination [0x10880C], back onto the source [0x108808] */
        f57CE8(S32(0x10880Cu), S32(0xF40F8u), 0, S32(0x108818u));   /* 0x40C57..0x40C6A (ebx = 0) */
        if (S32(0x108814u) == 1) {                                  /* 0x40C6F; 0x40C75 cmp ebx, 1; jne 0x40CA1 */
            /* 0x40C7F lea edx, [eax + ebx]: source + 1; 0x40C9B add byte [69(s+1) + n + 0xE31DC], dl */
            a = GAME_AREA + 69u * (uint32_t)(S32(0x108808u) + S32(0x108814u)) + (uint32_t)S32(0xF40F8u) + 0x24u;
            setD8(a, (uint8_t)(D8(a) + D8(0x108818u)));             /* 0x40C95 mov dl, byte [0x108818] */
        }
        f3F698(S32(0x108808u), 0x0E);                               /* 0x40CAB the source's token box, colour 0x0E */
        setD32(0xF06CCu, 1);                                        /* 0x40CB0 mov ecx, 1; 0x40CC6 mov [0xF06CC], ecx */
        f57E28(S32(0x108808u), S32(0xF40F8u), S32(0x108818u));      /* 0x40CB5..0x40CCC */
        n = S32(0xF40F8u);                                          /* 0x40CE5 */
        setD32(0x10881Cu, D32(0x10881Cu) - 1u);                     /* 0x40CEB mov esi, [0x10881C]; sub esi, ecx; 0x40CFD */
        /* 0x40CF1 dword [69([0x10880C]+1) + n + 0xE31D9] sar 24: the destination's movable tokens */
        if ((int32_t)S8(ar(S32(0x10880Cu), n, 0x24u)) == 0)         /* 0x40D03 test eax, eax; jne 0x40D0E */
            color = 5;                                              /* 0x40D07 */
        else
            color = 0x0F;                                           /* 0x40D0E */
        f3F698(S32(0x10880Cu), color);                              /* 0x40D18 */
        f3C3D0(MV_DLG, 0x28);                                       /* 0x40D27 UNDO MOVE disabled */
        f572CC(S32(0x108808u));                                     /* 0x40D31 */
        f3D52C(0, 0);                                               /* 0x40D3A */
    } else {
        /* the selected ship: from the destination [0x10880C] back to the source [0x108808] */
        f580CC(S32(0x10880Cu), S32(0x108808u), S32(0xF40F8u));      /* 0x40D44..0x40D55 */
        n = S32(0xF40F8u);                                          /* 0x40D6D */
        k = (int32_t)D8(ar(S32(0x108808u), n, 0x2Du));              /* 0x40D72 byte [.. + 0xE31E5]; and edx, 0xFF */
        a = nrec(n) + (uint32_t)k + 0x41u;                          /* 0x40D85 add edx, eax (199n) */
        setD8(a, (uint8_t)(D8(a) + 1u));                            /* 0x40D87 mov bl, [edx + 0xF150D]; inc bl; 0x40D94 */
        f576F8(S32(0x10880Cu));                                     /* 0x40D8F; 0x40D9A */
        setD32(0xF06CCu, 1);                                        /* 0x40D9F mov eax, 1; 0x40DA9 */
        f3F70C(S32(0x108808u), 0x0E);                               /* 0x40DB3 the source's ship box, colour 0x0E */
        f3C3D0(MV_DLG, 0x28);                                       /* 0x40DC2 UNDO MOVE disabled */
        f3F670();                                                   /* 0x40DC7 every border highlight off */
        f3F624(S32(0x108808u), 0x0F);                               /* 0x40DD6 the source's border */
        setD32(0x109E10u, D32(0x108808u));                          /* 0x40DDB; 0x40DE0 mov [0x109E10], eax */
        f426D0(S32(0x108808u));                                     /* 0x40DE5 (eax still [0x108808]) */
        f3DA60(S32(0x109E10u));                                     /* 0x40DEA; 0x40DEF centre the map on it */
    }
    f3CBBC();                                                       /* 0x40DF4 */
    if ((D8(0xE1624u) & 0x80) != 0)                                 /* 0x40DF9 test byte [0xE1624], 0x80; je */
        setD16(0xF09B8u, (uint16_t)(D16(0xF09B8u) - 1u));           /* 0x40E02 dec word [0xF09B8]: the move's log record */
    code = -4;                                                      /* 0x40E09 mov [ebp-0x3C], -4 */
    return code;                                                    /* 0x40E10 */
}

/* id 0x29 -> 0x40E1D FIND UNMOVED (the phase's -0x79 runs 0x57810) */
int dcase_29(int code)
{
    code = -0x79;                                                   /* 0x40E1D mov [ebp-0x3C], 0xFFFFFF87 */
    return code;                                                    /* 0x40E24 */
}

/* id 0x2F -> 0x40E45 EMBRK of the second coast: one token of the canonical territory onto the twin's selected ship */
int dcase_2F(int code)
{
    int32_t n;
    int32_t k;
    uint32_t a;

    f57CE8((int)D8(0xC803Cu + (uint32_t)S32(0x109E10u)), S32(0xF40F8u), 1, 1);   /* 0x40E45..0x40E62 (ebx = ecx = 1) */
    n = S32(0xF40F8u);                                              /* 0x40E80 */
    k = (int32_t)D8(ar(f3E89C(S32(0x109E10u)), n, 0x2Du));          /* 0x40E6C; 0x40E85 */
    a = nrec(S32(0xF40F8u)) + (uint32_t)k + 0x4Du;                  /* 0x40E8C imul edx, [0xF40F8], 0xC7; 0x40E9B */
    setD8(a, (uint8_t)(D8(a) + 1u));                                /* 0x40E9D mov cl, [eax + 0xF1519]; inc cl; 0x40EAB */
    f426D0(S32(0x109E10u));                                         /* 0x40EA5; 0x40EB3 */
    if ((int32_t)S8(0x108804u) == 1                                 /* 0x40EB8 dword [0x108801] sar 24; cmp eax, 1 */
        && (int32_t)S8(ar(S32(0x109E10u), S32(0xF40F8u), 0x24u)) == 0)   /* 0x40EDE dword [.. + 0xE31D9] sar 24 */
        setD32(0xF06CCu, 0);                                        /* 0x40EEC mov [0xF06CC], eax (0) */
    if ((D8(0xE1624u) & 0x80) != 0)                                 /* 0x40EF1 test byte [0xE1624], 0x80; je 0x40F5F */
        pbem_log(7, 1);                                             /* 0x40EFA..0x40F58 */
    f3C3D0(MV_DLG, 0x28);                                           /* 0x40F69 UNDO MOVE disabled */
    f3CBBC();                                                       /* 0x40F6E */
    return code;                                                    /* 0x40F73 mov eax, [ebp-0x3C] */
}

/* id 0x2B -> 0x40F80 EMBARK: one token of the canonical territory onto the selected ship of f3E8FC([0x109E10]) */
int dcase_2B(int code)
{
    int32_t i2;                                                     /* [ebp-0x20] */
    int32_t k;
    uint32_t a;

    f57CE8((int)D8(0xC803Cu + (uint32_t)S32(0x109E10u)), S32(0xF40F8u), 1, 1);   /* 0x40F80..0x40F9D */
    i2 = f3E8FC(S32(0x109E10u));                                    /* 0x40FA7; 0x40FAC */
    k = (int32_t)D8(ar(i2, S32(0xF40F8u), 0x2Du));                  /* 0x40FC4 */
    a = nrec(S32(0xF40F8u)) + (uint32_t)k + 0x4Du;                  /* 0x40FCB */
    setD8(a, (uint8_t)(D8(a) + 1u));                                /* 0x40FDA inc byte [edx + eax + 0xF1519] */
    f426D0(i2);                                                     /* 0x40FE4 */
    if ((int32_t)S8(0x108804u) == 1                                 /* 0x40FE9 dword [0x108801] sar 24; cmp eax, 1 */
        && (int32_t)S8(ar(i2, S32(0xF40F8u), 0x24u)) == 0)          /* 0x4100C sar 24; test */
        setD32(0xF06CCu, 0);                                        /* 0x4101A */
    if ((D8(0xE1624u) & 0x80) != 0)                                 /* 0x4101F */
        pbem_log(3, 1);                                             /* 0x41028..0x41086 */
    f3C3D0(MV_DLG, 0x28);                                           /* 0x41097 */
    f3CBBC();                                                       /* 0x4109C */
    return code;                                                    /* 0x410A1 */
}

/* id 0x30 -> 0x410AE DSMBRK of the second coast: one token off the twin's selected ship (see the file comment) */
int dcase_30(int code)
{
    int32_t n;
    int32_t k;
    int32_t t;                                                      /* [ebp-0x24] */
    uint32_t a;

    n = S32(0xF40F8u);                                              /* 0x410C2 edx; 0x410D4 esi */
    k = (int32_t)D8(ar(S32(0x109E10u), n, 0x2Du));                  /* 0x410AE..0x410CF the record of [0x109E10] */
    t = (int32_t)D8(0xC803Bu + (uint32_t)ship_lo(n, k));            /* 0x410E7 word, xor ah, ah; 0x410F6 [edx + 0xC803B] */
    f57E28(t, n, 1);                                                /* 0x41109 (ebx = 1, edx = esi) */
    n = S32(0xF40F8u);                                              /* 0x41133 edi */
    k = (int32_t)D8(ar(f3E89C(S32(0x109E10u)), S32(0xF40F8u), 0x2Du));   /* 0x41113; 0x4112C */
    a = nrec(n) + (uint32_t)k + 0x4Du;                              /* 0x41139..0x41144 */
    setD8(a, (uint8_t)(D8(a) - 1u));                                /* 0x41146 mov cl, [eax + 0xF1519]; dec cl; 0x41155 */
    k = (int32_t)D8(ar(S32(0x109E10u), n, 0x2Du));                  /* 0x4114C..0x41170 the record of [0x109E10] again */
    f426D0(ship_lo(n, k) - 1);                                      /* 0x41182 word; xor ah, ah; cwde; dec; 0x4118E */
    if ((D8(0xE1624u) & 0x80) != 0)                                 /* 0x41193 */
        pbem_log(8, 1);                                             /* 0x4119C..0x411FB */
    f3C3D0(MV_DLG, 0x28);                                           /* 0x4120C */
    f3CBBC();                                                       /* 0x41211 */
    return code;                                                    /* 0x41216 */
}

/* id 0x2C -> 0x41223 DISEMBARK: one token off the selected ship of f3E8FC([0x109E10]) into its canonical territory */
int dcase_2C(int code)
{
    int32_t i2;                                                     /* [ebp-0x20] */
    int32_t n;
    int32_t k;
    int32_t t;                                                      /* [ebp-0x24] */
    uint32_t a;

    i2 = f3E8FC(S32(0x109E10u));                                    /* 0x41228; 0x4122D */
    n = S32(0xF40F8u);                                              /* 0x41240; 0x41249 ecx */
    k = (int32_t)D8(ar(i2, n, 0x2Du));                              /* 0x4124F */
    t = (int32_t)D8(0xC803Bu + (uint32_t)ship_lo(n, k));            /* 0x4125B word; 0x4126A [edx + 0xC803B] */
    f57E28(t, n, 1);                                                /* 0x4127D (edx = ecx) */
    n = S32(0xF40F8u);                                              /* 0x41292; 0x4129C esi */
    k = (int32_t)D8(ar(i2, n, 0x2Du));                              /* 0x412A2 */
    a = nrec(n) + (uint32_t)k + 0x4Du;
    setD8(a, (uint8_t)(D8(a) - 1u));                                /* 0x412AE dec byte [edx + eax + 0xF1519] */
    k = (int32_t)D8(ar(i2, n, 0x2Du));                              /* 0x412C7 */
    f426D0(ship_lo(n, k) - 1);                                      /* 0x412E0 word; xor ah, ah; cwde; dec; 0x412EC */
    if ((D8(0xE1624u) & 0x80) != 0)                                 /* 0x412F1 */
        pbem_log(4, 1);                                             /* 0x412FA..0x41358 */
    f3C3D0(MV_DLG, 0x28);                                           /* 0x41369 */
    f3CBBC();                                                       /* 0x4136E */
    return code;                                                    /* 0x41373 */
}

/* id 0x31 -> 0x41380 DSMBRK-A of the second coast: every token off the twin's selected ship */
int dcase_31(int code)
{
    int32_t cnt = 0;                                                /* [ebp-0x40] (0x41382) */
    int32_t n;
    int32_t k;
    int32_t t;                                                      /* [ebp-0x24] */
    uint32_t a;

    n = S32(0xF40F8u);                                              /* 0x41399; 0x413A5 */
    k = (int32_t)D8(ar(S32(0x109E10u), n, 0x2Du));                  /* 0x41385..0x413AF the record of [0x109E10] */
    t = (int32_t)D8(0xC803Bu + (uint32_t)ship_lo(n, k));            /* 0x413B4 word; 0x413C3 [edx + 0xC803B] */
    for (;;) {                                                      /* 0x413CC */
        n = S32(0xF40F8u);                                          /* 0x413E5 edx */
        k = (int32_t)D8(ar(f3E89C(S32(0x109E10u)), n, 0x2Du));      /* 0x413D1; 0x413ED */
        if ((int32_t)S8(nrec(n) + (uint32_t)k + 0x4Du) == 0)        /* 0x413FA dword [k + 199n + 0xF1516] sar 24 */
            break;                                                  /* 0x41406 je 0x41458 */
        f57E28(t, n, 1);                                            /* 0x41410 (ebx = 1, edx still n) */
        n = S32(0xF40F8u);                                          /* 0x4142E */
        k = (int32_t)D8(ar(f3E89C(S32(0x109E10u)), n, 0x2Du));      /* 0x4141A; 0x41433 */
        a = nrec(n) + (uint32_t)k + 0x4Du;
        setD8(a, (uint8_t)(D8(a) - 1u));                            /* 0x41446 dec byte [edx + eax + 0xF1519] */
        cnt++;                                                      /* 0x41450 inc dword [ebp-0x40] */
    }
    n = S32(0xF40F8u);                                              /* 0x41471 */
    k = (int32_t)D8(ar(f3E89C(S32(0x109E10u)), n, 0x2Du));          /* 0x4145D; 0x41476 */
    f426D0(ship_lo(n, k) - 1);                                      /* 0x41489 word; xor ah, ah; cwde; dec; 0x41495 */
    if ((D8(0xE1624u) & 0x80) != 0)                                 /* 0x4149A */
        pbem_log(9, (uint8_t)cnt);                                  /* 0x414A3..0x41506 (+9 = byte [ebp-0x40]) */
    f3C3D0(MV_DLG, 0x28);                                           /* 0x41517 */
    f3CBBC();                                                       /* 0x4151C */
    return code;                                                    /* 0x41521 */
}

/* id 0x2D -> 0x4152E DISEMBARK ALL: every token off the selected ship of f3E8FC([0x109E10]) */
int dcase_2D(int code)
{
    int32_t cnt = 0;                                                /* [ebp-0x40] (0x41530) */
    int32_t i2;                                                     /* [ebp-0x20] */
    int32_t n;
    int32_t k;
    int32_t t;                                                      /* [ebp-0x24] */
    uint32_t a;

    i2 = f3E8FC(S32(0x109E10u));                                    /* 0x41538; 0x4153D */
    n = S32(0xF40F8u);                                              /* 0x41550; 0x41561 edi */
    k = (int32_t)D8(ar(i2, n, 0x2Du));                              /* 0x41555 */
    t = (int32_t)D8(0xC803Bu + (uint32_t)ship_lo(n, k));            /* 0x41574 word; 0x41583 [edx + 0xC803B] */
    for (;;) {                                                      /* 0x4158C */
        n = S32(0xF40F8u);                                          /* 0x4159C edx (ecx = 199n at 0x415A9) */
        k = (int32_t)D8(ar(i2, n, 0x2Du));                          /* 0x415A2 */
        if ((int32_t)S8(nrec(n) + (uint32_t)k + 0x4Du) == 0)        /* 0x415B4 dword [ecx + k + 0xF1516] sar 24 */
            break;                                                  /* 0x415C0 je 0x41606 */
        f57E28(t, n, 1);                                            /* 0x415CA (ebx = 1, edx = n) */
        n = S32(0xF40F8u);                                          /* 0x415DF */
        k = (int32_t)D8(ar(i2, n, 0x2Du));                          /* 0x415E4 */
        a = nrec(n) + (uint32_t)k + 0x4Du;
        setD8(a, (uint8_t)(D8(a) - 1u));                            /* 0x415F7 dec byte [edx + eax + 0xF1519] */
        cnt++;                                                      /* 0x41601 */
    }
    /* 0x41606..0x4162D: edx (n) and ecx (199n) as the loop's last test left them */
    k = (int32_t)D8(ar(i2, n, 0x2Du));                              /* 0x41616 byte [edx + eax + 0xE31E5] */
    f426D0(ship_lo(n, k) - 1);                                      /* 0x41622 word [ecx + 2*eax + 0xF1511]; cwde; dec */
    if ((D8(0xE1624u) & 0x80) != 0)                                 /* 0x41633 */
        pbem_log(5, (uint8_t)cnt);                                  /* 0x4163C..0x4169E */
    f3C3D0(MV_DLG, 0x28);                                           /* 0x416AF */
    f3CBBC();                                                       /* 0x416B4 */
    return code;                                                    /* 0x416B9 */
}

/* id 0x32 -> 0x416C6 NXT SHP of the second coast: the twin's selection to the next ship there */
int dcase_32(int code)
{
    uint32_t a;
    uint8_t v;

    a = ar(f3E89C(S32(0x109E10u)), S32(0xF40F8u), 0x2Du);           /* 0x416CB; 0x416DF */
    v = (uint8_t)(D8(a) + 1u);                                      /* 0x416E5 mov cl, [..]; inc cl */
    setD8(a, v);                                                    /* 0x416EE */
    setD8(ar(f3E89C(S32(0x109E10u)), S32(0xF40F8u), 0x2Du),         /* 0x41710; 0x41729 mov [..], bl */
          (uint8_t)((int32_t)v % 4));                               /* 0x41707 idiv ebx (4) */
    if (g_original) {
        /* BUG(1995) 1: the play-by-email log record is written and its count bumped without the PBEM test the other
           ship actions make (0x41730..0x4177A): outside PBEM the original writes through [0xF09B4] = 0 into low
           memory (move.c's file comment). */
        pbem_log_next(0x0C);
    } else if ((D8(0xE1624u) & 0x80) != 0) {                        /* the test of the other actions (0x40EF1) */
        pbem_log_next(0x0C);
    }
    f576F8(f3E89C(S32(0x109E10u)));                                 /* 0x41784; 0x41789 */
    f426D0(S32(0x109E10u));                                         /* 0x41793 */
    return code;                                                    /* 0x41798 */
}

/* id 0x2E -> 0x417A5 NEXT SHIP: the selection of [0x109E10] to the next ship there */
int dcase_2E(int code)
{
    uint32_t a;
    uint8_t v;
    int r;

    a = ar(S32(0x109E10u), S32(0xF40F8u), 0x2Du);                   /* 0x417A5..0x417B9 */
    v = (uint8_t)(D8(a) + 1u);                                      /* 0x417BF mov ch, [..]; inc ch */
    setD8(a, v);                                                    /* 0x417C8 */
    setD8(ar(S32(0x109E10u), S32(0xF40F8u), 0x2Du),                 /* 0x417E3..0x417FE mov [..], bl */
          (uint8_t)((int32_t)v % 4));                               /* 0x417E1 idiv ebx (4) */
    if (g_original) {
        /* BUG(1995) 1: as in 0x32, the log record and its count without the PBEM test (0x41805..0x41850) */
        pbem_log_next(0x0B);
    } else if ((D8(0xE1624u) & 0x80) != 0) {                        /* the test of the other actions */
        pbem_log_next(0x0B);
    }
    setD32(0xF06CCu, 0);                                            /* 0x41855 xor edx, edx; 0x4185C mov [0xF06CC], edx */
    f576F8(S32(0x109E10u));                                         /* 0x41857; 0x41862 */
    f426D0(S32(0x109E10u));                                         /* 0x41871 (ebx = 0x0F, for 0x576A8) */
    r = f576A8(S32(0x109E10u), S32(0xF40F8u), 0x0F);                /* 0x41876..0x41881 */
    f3F70C(S32(0x109E10u), r);                                      /* 0x41886 mov edx, eax; 0x41892 the ship box */
    code = -2;                                                      /* 0x4188D mov ebx, -2; 0x41897 mov [ebp-0x3C], ebx */
    return code;                                                    /* 0x4189A */
}
