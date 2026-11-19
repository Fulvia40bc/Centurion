/* Module STK of docs/M6_PLAN.md: the World State Stack's peek and drop, the Move List Stack, and three fixed
   snapshots of AC_MAIN.EXE:
     0x7A688 / 0x7A700 world snapshot and back, 0x7A778 / 0x7A7B0 nations snapshot and back, 0x7A7E8 / 0x7A85C
     move-list snapshot and back, 0x7AA1C Move List Stack push, 0x7AD24 World State Stack drop, 0x7AD7C Move List
     Stack drop, 0x7AE14 Move List Stack pop, 0x7AF28 World State Stack peek, 0x7AFB8 Move List Stack peek.
   stack.c has the rest of the World State Stack (0x7A8D0 allocate, 0x7A940 push, 0x7ABD0 free, 0x7AC60 pop) and its
   layout. Literal translations; the address of each instruction a line comes from is in its comment. D = read in the
   code, G = inference. Prototypes: mstack.h.

   The two move lists (D; G: what they hold is the movement code's business):
     0x107694  list 1, 4-byte entries, int32 count [0x1079B4] (0x320 bytes before the count: 200 entries)
     0x1079B8  list 2, 6-byte entries, int32 count [0x107D3C] (0x384 bytes: 150 entries)
   The Move List Stack (D):
     int32 [0xCAC58]   depth, -1 = empty (initialised data: 0xFFFFFFFF in obj2); slot d is the d-th push (0..6)
     int32 [0x112A90]  the deepest it has been (BSS; G: printed by the developer statistics "Max MoveStack: %d",
                       0xB0F9E, beside [0x112A8C], the World State Stack's)
     uint32 0x112A1C[7]  handles of slot d's copy of list 1 (4 * count bytes), 0 when none
     uint32 0x112A38[7]  handles of slot d's copy of list 2 (6 * count bytes), 0 when none
     int32  0x112A54[7]  slot d's count 1;  int32 0x112A70[7]  slot d's count 2
   The blocks are port heap handles (load_heap_alloc / load_heap_ptr / load_heap_free), never C pointers (M6_PLAN
   risk 10). A slot whose count is <= 0 gets no block and its handle field is not written (it is 0: every pop or drop
   zeroes the handles it frees, and the BSS starts at 0, D).
   The fixed snapshots (D): 0x117CC0 (0x2B20), 0x117AE0 (0x1E0), 0x116518 (0x88D) for 0x7A688; 0x116FFC (0x88D) for
   0x7A778; 0x11A828 (800) with count [0x11AB48] and 0x11AB4C (900) with count [0x11AED0] for 0x7A7E8. The world ranges
   are those of the World State Stack: areas 1..160 at 0xE31FD, the 160 territory-state records at 0xE2C18, nations
   1..11 at 0xF1593.

   Runtime calls (Watcom C runtime -> standard C): 0x822B5 malloc -> load_heap_alloc; 0x877E6 free -> load_heap_free
   (0 and dead handles ignored, as free(NULL)); 0x7DB3D sprintf -> snprintf bounded to 0xF5500's 0x1F4 bytes as
   elsewhere in the port, with the original's format from the data object; the inline byte-pair strcpy of the three
   "popped" texts -> the same bounded snprintf("%s"); the inline rep movsd / movsb copies -> memcpy (fixed sizes) or a
   forward byte copy (sizes from a count: the same result as rep movsd/movsb even if the ranges overlapped by 4 or more
   bytes). __CHK dropped. f44CAC (quit) does not return in the port; in the original each error path returns after it.

   Port guards (G): a copy whose bytes lie outside the data object (a count the original copies unchecked, so a
   negative or huge one) or a slot handle that does not resolve to a block of the size needed (a stack read where
   nothing was pushed, or malloc failed) would corrupt or read outside the original's memory; the port puts a message
   into 0xF5500 and quits instead (the source of a copy is checked before its destination). Counts within the lists'
   sizes (200 and 150) never trip them.

   Text slips (fixed unless g_original, docs/TEXT_SWEEP.md): "popped once to often" -> "too often" in the three copies
   here (0xB96A5, 0xB96D1, 0xB96F6). "Move Stack" for "Move List Stack" is wording, kept.
   Oddities kept (D):
     - The peeks 0x7AF28 and 0x7AFB8 do not test for an empty stack: at depth -1 they read the field before each array
       (0xF523C, 0xF5264 = slot 9's area handle, 0xF528C = slot 9's nation handle; 0x112A18, 0x112A34 = 0x112A1C[6],
       0x112A50 = 0x112A38[6] read as count 1, 0x112A6C = 0x112A54[6] read as count 2). G: the callers push first;
       f617C0 (ship construction) peeks at 0x617DE without a push of its own, so its callers must have pushed (to
       check when SHIP is ported). The port copies the same bytes when the fields resolve, else the guard quits.
     - The drop 0x7AD7C zeroes slot d's list-2 handle and count through [4*(d-1) + 0x112A3C] and [4*(d-1) + 0x112A74]
       (0x7ADCA, 0x7ADD1): the same addresses as 0x112A38[d] and 0x112A70[d].
     - The pop 0x7AE14 leaves the slot's counts in 0x112A54 / 0x112A70; the drop zeroes them. Nothing reads a slot
       above the depth (the push writes both counts first), so the difference shows only in a dump.
     - The push's "> %d" prints 7, the slot count; it refuses at depth 6 (all seven slots in use). Consistent, like
       the World State Stack's 10 (the inventory record's "limit is 6 pushes" counts from depth 0: from the empty
       stack, -1, seven pushes fit). */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "mstack.h"
#include "state.h"
#include "ui.h"

#define WS_DEPTH   0xCAC54u    /* int32 the World State Stack's depth, -1 = empty */
#define WS_AREA    0xF5240u    /* uint32[10] handles of 0x2B20-byte blocks (stack.c) */
#define WS_NAT     0xF5268u    /* uint32[10] handles of 0x88D-byte blocks */
#define WS_TST     0xF5290u    /* uint32[10] handles of 0x1E0-byte blocks */
#define ML_DEPTH   0xCAC58u    /* int32 the Move List Stack's depth, -1 = empty */
#define ML_MAX     0x112A90u   /* int32 the deepest it has been */
#define ML_PTR1    0x112A1Cu   /* uint32[7] handles of the list-1 copies */
#define ML_PTR2    0x112A38u   /* uint32[7] handles of the list-2 copies */
#define ML_CNT1    0x112A54u   /* int32[7] count 1 of each slot */
#define ML_CNT2    0x112A70u   /* int32[7] count 2 of each slot */
#define LIST1      0x107694u   /* move list 1, 4-byte entries */
#define COUNT1     0x1079B4u   /* int32 its count */
#define LIST2      0x1079B8u   /* move list 2, 6-byte entries */
#define COUNT2     0x107D3Cu   /* int32 its count */
#define AREA1      0xE31FDu    /* area record 1 (0xE31B8 + 69) */
#define NAT1       0xF1593u    /* nation record 1 (0xF14CC + 199) */
#define TSTATE     0xE2C18u    /* the 160 territory-state records */
#define SZ_AREA    0x2B20u
#define SZ_NAT     0x88Du
#define SZ_TST     0x1E0u
#define MSG        0xF5500u    /* the quit message buffer */

/* ---- port guards (file comment) ------------------------------------------------------------------------------ */

/* n bytes at link address a of the data object, or quit. */
static uint8_t *ds_range(uint32_t a, uint32_t n)
{
    if (!ds_valid(a, n)) {
        snprintf((char *)DP(MSG), 0x1F4, "Stack copy of 0x%X bytes at 0x%X is outside the data object", (unsigned)n,
                 (unsigned)a);
        f44CAC();
    }
    return DP(a);
}

/* n bytes of the block whose handle the field holds, or quit. */
static uint8_t *slot_block(uint32_t field, uint32_t n)
{
    uint8_t *p = load_heap_ptr(D32(field), n);

    if (p == NULL) {
        snprintf((char *)DP(MSG), 0x1F4, "Stack slot 0x%X has no block of 0x%X bytes", (unsigned)field, (unsigned)n);
        f44CAC();
    }
    return p;
}

/* rep movsd / rep movsb of n bytes, forward */
static void copy_fwd(uint8_t *dst, const uint8_t *src, uint32_t n)
{
    uint32_t i;

    for (i = 0; i < n; i++)
        dst[i] = src[i];
}

/* n bytes from link address src to link address dst (source checked first) */
static void ds_copy(uint32_t dst, uint32_t src, uint32_t n)
{
    const uint8_t *s = ds_range(src, n);

    copy_fwd(ds_range(dst, n), s, n);
}

/* n bytes from link address src into the block of slot field */
static void to_slot(uint32_t field, uint32_t src, uint32_t n)
{
    const uint8_t *s = ds_range(src, n);

    copy_fwd(slot_block(field, n), s, n);
}

/* n bytes from the block of slot field to link address dst */
static void from_slot(uint32_t dst, uint32_t field, uint32_t n)
{
    const uint8_t *s = slot_block(field, n);

    copy_fwd(ds_range(dst, n), s, n);
}

/* The inline byte-pair strcpy of a "popped once to often." text into 0xF5500, then the quit. */
static _Noreturn void popped(uint32_t text, const char *fixed)
{
    snprintf((char *)DP(MSG), 0x1F4, "%s", g_original ? DSTR(text) : fixed);
    f44CAC();
}

/* ---- 0x7A688 / 0x7A700: world snapshot and back --------------------------------------------------------------- */

void f7A688(void)
{
    memcpy(DP(0x117CC0u), DP(AREA1), SZ_AREA);                      /* 0x7A698..0x7A6B4 */
    memcpy(DP(0x117AE0u), DP(TSTATE), SZ_TST);                      /* 0x7A6B7..0x7A6D3 */
    memcpy(DP(0x116518u), DP(NAT1), SZ_NAT);                        /* 0x7A6D6..0x7A6F2 */
}                                                                   /* 0x7A6FC */

void f7A700(void)
{
    memcpy(DP(AREA1), DP(0x117CC0u), SZ_AREA);                      /* 0x7A710..0x7A72C */
    memcpy(DP(TSTATE), DP(0x117AE0u), SZ_TST);                      /* 0x7A72F..0x7A74B */
    memcpy(DP(NAT1), DP(0x116518u), SZ_NAT);                        /* 0x7A74E..0x7A76A */
}                                                                   /* 0x7A774 */

/* ---- 0x7A778 / 0x7A7B0: nations snapshot and back ------------------------------------------------------------- */

void f7A778(void)
{
    memcpy(DP(0x116FFCu), DP(NAT1), SZ_NAT);                        /* 0x7A788..0x7A7A4 */
}                                                                   /* 0x7A7AE */

void f7A7B0(void)
{
    memcpy(DP(NAT1), DP(0x116FFCu), SZ_NAT);                        /* 0x7A7C0..0x7A7DC */
}                                                                   /* 0x7A7E6 */

/* ---- 0x7A7E8 / 0x7A85C: move-list snapshot and back ----------------------------------------------------------- */

void f7A7E8(void)
{
    uint32_t c, n;

    c = D32(COUNT1);                                                /* 0x7A7FD eax = [0x1079B4] */
    setD32(0x11AB48u, c);                                           /* 0x7A807 */
    n = 4u * c;                                                     /* 0x7A80C lea ecx, [4*eax] (no test) */
    ds_copy(0x11A828u, LIST1, n);                                   /* 0x7A813..0x7A820 */
    c = D32(COUNT2);                                                /* 0x7A823 eax = [0x107D3C] */
    setD32(0x11AED0u, c);                                           /* 0x7A828 */
    n = (4u * c - c) * 2u;                                          /* 0x7A82D lea; 0x7A839 sub; 0x7A840 add: 6 * c */
    ds_copy(0x11AB4Cu, LIST2, n);                                   /* 0x7A842..0x7A84F */
}                                                                   /* 0x7A859 */

void f7A85C(void)
{
    uint32_t c, n;

    c = D32(0x11AB48u);                                             /* 0x7A871 */
    setD32(COUNT1, c);                                              /* 0x7A87B */
    n = 4u * c;                                                     /* 0x7A880 */
    ds_copy(LIST1, 0x11A828u, n);                                   /* 0x7A887..0x7A894 */
    c = D32(0x11AED0u);                                             /* 0x7A897 */
    setD32(COUNT2, c);                                              /* 0x7A89C */
    n = (4u * c - c) * 2u;                                          /* 0x7A8A1..0x7A8B4: 6 * c */
    ds_copy(LIST2, 0x11AB4Cu, n);                                   /* 0x7A8B6..0x7A8C3 */
}                                                                   /* 0x7A8CD */

/* ---- 0x7AA1C: Move List Stack push ---------------------------------------------------------------------------- */

void f7AA1C(void)
{
    int32_t d = S32(ML_DEPTH);                                      /* 0x7AA2E edx = [0xCAC58] */
    int32_t nd, c;
    uint32_t h, n;

    if (d >= 6) {                                                   /* 0x7AA34 cmp edx, 6; jge 0x7AAFE (signed) */
        snprintf((char *)DP(MSG), 0x1F4, DSTR(0xB964Cu), 7);        /* 0x7AB0A sprintf(0xF5500, "Move List Stack has
                                                                       blown its max size. > %d", 7) */
        f44CAC();                                                   /* 0x7AB12 quit; the original returns after it */
    }
    nd = d + 1;                                                     /* 0x7AA3D lea ebx, [edx+1] */
    setD32(ML_DEPTH, (uint32_t)nd);                                 /* 0x7AA46 */
    if (nd > S32(ML_MAX))                                           /* 0x7AA4C cmp ebx, ecx; jle (signed) */
        setD32(ML_MAX, (uint32_t)nd);                               /* 0x7AA50 */

    d = S32(ML_DEPTH);                                              /* 0x7AA56 */
    c = S32(COUNT1);                                                /* 0x7AA5C */
    setD32(ML_CNT1 + 4u * (uint32_t)d, (uint32_t)c);                /* 0x7AA61 */
    if (c > 0) {                                                    /* 0x7AA68 test eax, eax; jle 0x7AAA1 */
        h = load_heap_alloc((uint32_t)c << 2);                      /* 0x7AA6C shl eax, 2; 0x7AA74 malloc */
        d = S32(ML_DEPTH);                                          /* 0x7AA79 */
        n = D32(COUNT1) << 2;                                       /* 0x7AA7F, 0x7AA87 */
        setD32(ML_PTR1 + 4u * (uint32_t)d, h);                      /* 0x7AA8A */
        to_slot(ML_PTR1 + 4u * (uint32_t)d, LIST1, n);              /* 0x7AA91..0x7AA9E */
    }
    d = S32(ML_DEPTH);                                              /* 0x7AAA1 eax = [0xCAC58] */
    c = S32(COUNT2);                                                /* 0x7AAA6 */
    setD32(ML_CNT2 + 4u * (uint32_t)d, (uint32_t)c);                /* 0x7AAAC */
    if (c > 0) {                                                    /* 0x7AAB3 test edx, edx; jle 0x7AB17 */
        h = load_heap_alloc(((uint32_t)c * 4u - (uint32_t)c) * 2u); /* 0x7AAB7..0x7AAC0 6 * count; 0x7AAC2 malloc */
        n = (D32(COUNT2) * 4u - D32(COUNT2)) * 2u;                  /* 0x7AAC7, 0x7AAD2, 0x7AADF, 0x7AAE3 */
        d = S32(ML_DEPTH);                                          /* 0x7AAD9 */
        setD32(ML_PTR2 + 4u * (uint32_t)d, h);                      /* 0x7AAE5 */
        to_slot(ML_PTR2 + 4u * (uint32_t)d, LIST2, n);              /* 0x7AAEC..0x7AAF9 */
    }
}                                                                   /* 0x7AB20 */

/* ---- 0x7AD24: World State Stack drop -------------------------------------------------------------------------- */

void f7AD24(void)
{
    int32_t d = S32(WS_DEPTH);                                      /* 0x7AD35 edx = [0xCAC54] */

    if (d < 0)                                                      /* 0x7AD3B test edx, edx; jl 0x7AD4A */
        popped(0xB96A5u, "World State Stack has popped once too often.");   /* 0x7AD4A..0x7AD6E (text slip fixed) */
    setD32(WS_DEPTH, (uint32_t)(d - 1));                            /* 0x7AD3F lea ebx, [edx-1]; 0x7AD42 */
}                                                                   /* 0x7AD7B */

/* ---- 0x7AD7C: Move List Stack drop ---------------------------------------------------------------------------- */

void f7AD7C(void)
{
    int32_t d = S32(ML_DEPTH);                                      /* 0x7AD8D edx = [0xCAC58] */

    if (d < 0)                                                      /* 0x7AD93 test edx, edx; jl 0x7ADDF */
        popped(0xB96D1u, "Move Stack has popped once too often.");  /* 0x7ADDF..0x7AE03 (text slip fixed) */
    load_heap_free(D32(ML_PTR1 + 4u * (uint32_t)d));                /* 0x7AD97, 0x7AD9E free */
    d = S32(ML_DEPTH);                                              /* 0x7ADA3 */
    setD32(ML_PTR1 + 4u * (uint32_t)d, 0);                          /* 0x7ADAA (ebx = 0) */
    setD32(ML_CNT1 + 4u * (uint32_t)d, 0);                          /* 0x7ADB1 */
    load_heap_free(D32(ML_PTR2 + 4u * (uint32_t)d));                /* 0x7ADB8, 0x7ADBF free */
    d = S32(ML_DEPTH) - 1;                                          /* 0x7ADC4, 0x7ADC9 dec eax */
    setD32(0x112A3Cu + 4u * (uint32_t)d, 0);                        /* 0x7ADCA [4*(d-1) + 0x112A3C] = 0x112A38[d] */
    setD32(0x112A74u + 4u * (uint32_t)d, 0);                        /* 0x7ADD1 [4*(d-1) + 0x112A74] = 0x112A70[d] */
    setD32(ML_DEPTH, (uint32_t)d);                                  /* 0x7ADD8 */
}                                                                   /* 0x7AE10 */

/* ---- 0x7AE14: Move List Stack pop ----------------------------------------------------------------------------- */

void f7AE14(void)
{
    int32_t d = S32(ML_DEPTH);                                      /* 0x7AE25 edx = [0xCAC58] */
    int32_t c;
    uint32_t n;

    if (d < 0)                                                      /* 0x7AE2B test edx, edx; jl 0x7AEF6 */
        popped(0xB96F6u, "Move Stack has popped once too often.");  /* 0x7AEF6..0x7AF1A (text slip fixed) */
    c = S32(ML_CNT1 + 4u * (uint32_t)d);                            /* 0x7AE33 esi = 4d; 0x7AE3A */
    setD32(COUNT1, (uint32_t)c);                                    /* 0x7AE40 */
    if (c > 0) {                                                    /* 0x7AE45 test eax, eax; jle 0x7AE8A */
        n = (uint32_t)c * 4u;                                       /* 0x7AE4E lea ecx, [4*eax] */
        from_slot(LIST1, ML_PTR1 + 4u * (uint32_t)d, n);            /* 0x7AE49..0x7AE68 */
        d = S32(ML_DEPTH);                                          /* 0x7AE6B */
        load_heap_free(D32(ML_PTR1 + 4u * (uint32_t)d));            /* 0x7AE70, 0x7AE77 free */
        d = S32(ML_DEPTH);                                          /* 0x7AE7C */
        setD32(ML_PTR1 + 4u * (uint32_t)d, 0);                      /* 0x7AE83 (ecx = 0) */
    }
    d = S32(ML_DEPTH);                                              /* 0x7AE8A ecx = [0xCAC58] */
    c = S32(ML_CNT2 + 4u * (uint32_t)d);                            /* 0x7AE93 */
    setD32(COUNT2, (uint32_t)c);                                    /* 0x7AE99 */
    if (c > 0) {                                                    /* 0x7AE9E test eax, eax; jle 0x7AEE7 */
        n = ((uint32_t)c * 4u - (uint32_t)c) * 2u;                  /* 0x7AEA2..0x7AEAE 6 * count */
        from_slot(LIST2, ML_PTR2 + 4u * (uint32_t)d, n);            /* 0x7AEB0..0x7AEC5 */
        d = S32(ML_DEPTH);                                          /* 0x7AEC8 */
        load_heap_free(D32(ML_PTR2 + 4u * (uint32_t)d));            /* 0x7AECD, 0x7AED4 free */
        d = S32(ML_DEPTH);                                          /* 0x7AED9 */
        setD32(ML_PTR2 + 4u * (uint32_t)d, 0);                      /* 0x7AEE0 (edi = 0) */
    }
    setD32(ML_DEPTH, (uint32_t)(S32(ML_DEPTH) - 1));                /* 0x7AEE7 dec [0xCAC58] */
}                                                                   /* 0x7AEF5 */

/* ---- 0x7AF28: World State Stack peek -------------------------------------------------------------------------- */

void f7AF28(void)
{
    int32_t d;

    d = S32(WS_DEPTH);                                              /* 0x7AF38 (no test) */
    memcpy(DP(AREA1), slot_block(WS_AREA + 4u * (uint32_t)d, SZ_AREA), SZ_AREA);   /* 0x7AF3E..0x7AF5C */
    d = S32(WS_DEPTH);                                              /* 0x7AF5F */
    memcpy(DP(TSTATE), slot_block(WS_TST + 4u * (uint32_t)d, SZ_TST), SZ_TST);     /* 0x7AF65..0x7AF83 */
    d = S32(WS_DEPTH);                                              /* 0x7AF86 */
    memcpy(DP(NAT1), slot_block(WS_NAT + 4u * (uint32_t)d, SZ_NAT), SZ_NAT);       /* 0x7AF8C..0x7AFAA */
}                                                                   /* 0x7AFB4 */

/* ---- 0x7AFB8: Move List Stack peek ---------------------------------------------------------------------------- */

void f7AFB8(void)
{
    int32_t d, c;
    uint32_t n;

    d = S32(ML_DEPTH);                                              /* 0x7AFC8 (no test); 0x7AFCD shl eax, 2 */
    c = S32(ML_CNT1 + 4u * (uint32_t)d);                            /* 0x7AFD0 */
    setD32(COUNT1, (uint32_t)c);                                    /* 0x7AFD6 */
    if (c > 0) {                                                    /* 0x7AFDC test ecx, ecx; jle 0x7AFFE */
        n = (uint32_t)c << 2;                                       /* 0x7AFE5 shl ecx, 2 */
        from_slot(LIST1, ML_PTR1 + 4u * (uint32_t)d, n);            /* 0x7AFE0..0x7AFFB */
    }
    d = S32(ML_DEPTH);                                              /* 0x7AFFE; 0x7B004 shl esi, 2 */
    c = S32(ML_CNT2 + 4u * (uint32_t)d);                            /* 0x7B007 */
    setD32(COUNT2, (uint32_t)c);                                    /* 0x7B00D */
    if (c > 0) {                                                    /* 0x7B012 test eax, eax; jle 0x7B03B */
        n = ((uint32_t)c * 4u - (uint32_t)c) * 2u;                  /* 0x7B016..0x7B028 6 * count */
        from_slot(LIST2, ML_PTR2 + 4u * (uint32_t)d, n);            /* 0x7B01B..0x7B038 */
    }
}                                                                   /* 0x7B042 */
