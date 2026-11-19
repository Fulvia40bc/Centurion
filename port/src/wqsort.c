/* Watcom C/C++32 (1994) qsort, 0x8C331 in AC_MAIN.EXE (library code, 1,085 bytes), with its two helpers:
     0x8C2B3 the byte/dword exchange of two element blocks (also inlined twice in 0x8C331, at 0x8C67B and 0x8C6C3,
             for the two block moves after the partition)
     0x8C2D9 the median of three (returns one of its three element pointers)
   Translated literally: same control flow, comparisons with the original's signedness, the explicit stack of 32
   (base, count) pairs in the frame ([ebp-0x144] / [ebp-0xC4]) instead of recursion. The address of each instruction a
   line comes from is in its comment. Grades: D = read in the code, G = inference. docs/LIBRARY_CALLS.md has the same
   reading, made and checked independently.

   Registers and locals (D): eax base -> [ebp-0x148], edx n -> [ebp-0x14C], ebx width w -> [ebp-0x150], ecx compare ->
   [ebp-0x14]; [ebp-0x1C] swap mode; [ebp-0x38] 3w, [ebp-0x34] 2w; [ebp-0x10] stack depth. The comparator is called
   with eax, edx = the two element pointers (a Watcom register function, cmp(a, b) in C).

   The algorithm (D):
     - mode = 2 if (base | w) & 3, else 1 if w > 4 (unsigned), else 0. Modes 1 and 2 swap through 0x8C2B3 and put the
       pivot into a[0]; mode 0 (w == 4 on a 4-aligned base) swaps dwords and compares with a copy of the pivot ([ebp-
       0x44]). So the order of ties depends on the base's alignment, as in the original (wqsort.h).
     - n <= 1 (unsigned): pop. n < 16: insertion sort with gap 3w, then w (gap -= 2w while gap > 0, signed): for i =
       base + gap, base + 2gap, ... < end; j = i; while j > base and cmp(j - gap, j) > 0: swap, j -= gap. Then pop.
     - n >= 16: pivot = the middle a[n/2]; n > 29: median of first, middle, last; n > 42: the ninther with d = (n/8)w.
       Partition (Bentley-McIlroy, equal keys gathered at both ends), the two equal runs moved to the middle, then the
       larger part pushed and the smaller continued; when the right part is smaller and the left has at most one
       element, pop (the right part is dropped: it is then 0 or 1 elements too).
   Pointers are held as uintptr_t "registers" (the original's 32-bit unsigned compares), because the partition can
   move pc one element below base (when the middle pivot of 16..29 elements is the strict minimum in mode 0): no C
   pointer is formed there, only the value compared.

   Port notes: n and w are 32-bit in the original (size_t truncated to 32 bits here, as the game's values are small).
   w == 0 with n >= 16 divides by zero at 0x8C706, as the original's div would fault (G: never called so). */

#include <stdint.h>
#include <string.h>

#include "wqsort.h"

typedef int (*wc_cmp_fn)(const void *, const void *);

#define PTR(a) ((const void *)(a))

/* 0x8C2B3: exchange ecx bytes at [esi] and [edi]: ecx >> 2 dwords (xchg [esi], stosd), then cl & 3 bytes */
static void f8C2B3(uintptr_t esi, uintptr_t edi, uint32_t ecx)
{
    uint32_t edx = ecx & 0xFF;                      /* 0x8C2B6 movzx edx, cl */
    unsigned char *s = (unsigned char *)esi, *d = (unsigned char *)edi;
    unsigned char t4[4], t;

    ecx >>= 2;                                      /* 0x8C2B9 shr ecx, 2; je */
    while (ecx != 0) {                              /* 0x8C2BE..0x8C2C7 */
        memcpy(t4, d, 4);                           /* mov eax, [edi] */
        memcpy(d, s, 4);                            /* xchg [esi], eax; stosd */
        memcpy(s, t4, 4);
        s += 4;                                     /* add esi, 4 (stosd: edi += 4) */
        d += 4;
        ecx--;
    }
    edx &= 3;                                       /* 0x8C2C9 and dl, 3; je */
    while (edx != 0) {                              /* 0x8C2CE..0x8C2D5 */
        t = *d;
        *d = *s;
        *s = t;
        s++;
        d++;
        edx--;
    }
}

/* the dword swap of mode 0 (0x8C3D8, 0x8C528 dead, 0x8C58E, 0x8C5DA, 0x8C61F): mov eax,[x]; mov edx,[y]; swap */
static void swap4(uintptr_t x, uintptr_t y)
{
    unsigned char a[4], b[4];

    memcpy(a, (const void *)x, 4);
    memcpy(b, (const void *)y, 4);
    memcpy((void *)x, b, 4);
    memcpy((void *)y, a, 4);
}

/* 0x8C2D9: median of three: eax a, edx b, ebx c, ecx cmp; returns a, b or c */
static uintptr_t f8C2D9(uintptr_t a, uintptr_t b, uintptr_t c, wc_cmp_fn cmp)
{
    if (cmp(PTR(a), PTR(b)) > 0) {                  /* 0x8C2EB; 0x8C2F0 jle 0x8C310 */
        if (cmp(PTR(a), PTR(c)) > 0) {              /* 0x8C2F6; 0x8C2FB jle 0x8C30C */
            if (cmp(PTR(b), PTR(c)) > 0)            /* 0x8C301; 0x8C306 jg 0x8C326 */
                return b;                           /* 0x8C326 */
            return c;                               /* 0x8C308 */
        }
        return a;                                   /* 0x8C30C */
    }
    if (cmp(PTR(a), PTR(c)) >= 0)                   /* 0x8C314; 0x8C319 jge 0x8C30C */
        return a;
    if (cmp(PTR(b), PTR(c)) > 0)                    /* 0x8C31F; 0x8C324 jg 0x8C308 */
        return c;
    return b;                                       /* 0x8C326 */
}

/* 0x8C331 */
void wc_qsort(void *base_, size_t n_, size_t size_, int (*cmp)(const void *, const void *))
{
    uintptr_t base = (uintptr_t)base_;              /* [ebp-0x148] (push eax) */
    uint32_t n = (uint32_t)n_;                      /* [ebp-0x14C] (push edx) */
    uint32_t w = (uint32_t)size_;                   /* [ebp-0x150] (push ebx) */
    int32_t mode;                                   /* [ebp-0x1C] */
    int32_t gap3, step2;                            /* [ebp-0x38], [ebp-0x34] */
    int32_t gap;                                    /* [ebp-0x18] */
    uint32_t sp;                                    /* [ebp-0x10] */
    uintptr_t stk_base[32];                         /* [ebp-0x144] */
    uint32_t stk_n[32];                             /* [ebp-0xC4] */
    uint32_t tmp;                                   /* [ebp-0x44]: the pivot's copy in mode 0 */
    uintptr_t piv;                                  /* [ebp-0x2C] */
    uintptr_t lo;                                   /* [ebp-0x30] */
    uint32_t d, d2;                                 /* [ebp-0x20], [ebp-0x40] */
    uintptr_t i, end;                               /* [ebp-0x24], [ebp-0x3C] */
    uintptr_t pa, pc, pd, eend;                     /* [ebp-0xC], [ebp-0x4], [ebp-0x8], [ebp-0x28] */
    uintptr_t ebx, esi, edi, ecx;
    int32_t sr, st;
    uint32_t ur, ut, left, right;
    int r;

    if (((base | w) & 3) != 0)                      /* 0x8C33F or eax, ebx; test al, 3; je */
        mode = 2;                                   /* 0x8C348 */
    else
        mode = w > 4;                               /* 0x8C34F cmp ebx, 4; seta */
    gap3 = (int32_t)(w * 3);                        /* 0x8C35B imul eax, [ebp-0x150], 3 */
    step2 = (int32_t)(w + w);                       /* 0x8C365 add eax, eax */
    sp = 0;                                         /* 0x8C36D */

    for (;;) {                                      /* 0x8C377 */
        if (n <= 1)                                 /* 0x8C377 cmp 1; jbe 0x8C415 */
            goto pop;
        if (n < 0x10) {                             /* 0x8C384 cmp 0x10; jae 0x8C444 */
            gap = gap3;                             /* 0x8C394 */
            end = base + (uintptr_t)(n * w);        /* 0x8C397..0x8C3AC */
            while (gap > 0) {                       /* 0x8C40F cmp 0; jg 0x8C3B1 */
                for (i = base + (uintptr_t)gap; i < end; i += (uintptr_t)gap) {   /* 0x8C3B1; 0x8C401 jb; 0x8C3FB */
                    ebx = i;                        /* 0x8C3BF */
                    while (ebx > base) {            /* 0x8C3E3 cmp ebx, base; jbe 0x8C3FB */
                        edi = ebx - (uintptr_t)gap; /* 0x8C3EB */
                        if (cmp(PTR(edi), PTR(ebx)) <= 0)   /* 0x8C3F4; 0x8C3F9 jg 0x8C3C3 */
                            break;
                        if (mode != 0)              /* 0x8C3C3 */
                            f8C2B3(ebx, edi, w);    /* 0x8C3D1 esi = ebx */
                        else
                            swap4(edi, ebx);        /* 0x8C3D8 */
                        ebx -= (uintptr_t)gap;      /* 0x8C3E0 */
                    }
                }
                gap -= step2;                       /* 0x8C409 */
            }
            goto pop;                               /* falls into 0x8C415 */
        }

        /* n >= 16: the pivot */
        edi = base + (uintptr_t)((n >> 1) * w);     /* 0x8C444..0x8C453 shr; imul; add */
        if (n > 0x1D) {                             /* 0x8C459 cmp 0x1D; jbe 0x8C508 */
            lo = base;                              /* 0x8C46C */
            esi = base + (uintptr_t)((n - 1) * w);  /* 0x8C46F..0x8C483 */
            if (n > 0x2A) {                         /* 0x8C485 cmp 0x2A; jbe 0x8C4F7 */
                d = (n >> 3) * w;                   /* 0x8C49A shr 3; imul; 0x8C4A9 [ebp-0x20] */
                d2 = d + d;                         /* 0x8C4AC lea; 0x8C4B5 [ebp-0x40] */
                lo = f8C2D9(base, base + d, base + d2, cmp);            /* 0x8C4C3 */
                edi = f8C2D9(edi - d, edi, edi + d, cmp);               /* 0x8C4DA */
                esi = f8C2D9(esi - d2, esi - d, esi, cmp);              /* 0x8C4F0 */
            }
            edi = f8C2D9(lo, edi, esi, cmp);        /* 0x8C501 */
        }
        if (mode != 0) {                            /* 0x8C508 cmp [ebp-0x1C], 0; je 0x8C538 */
            piv = base;                             /* 0x8C514 */
            /* 0x8C517 je 0x8C528: the flags are still those of 0x8C508, never taken; 0x8C528 is dead code */
            f8C2B3(base, edi, w);                   /* 0x8C521 esi = base, edi = the pivot */
        } else {
            piv = (uintptr_t)&tmp;                  /* 0x8C538 lea eax, [ebp-0x44] */
            memcpy(&tmp, PTR(edi), 4);              /* 0x8C53E mov eax, [edi]; mov [ebp-0x44], eax */
        }

        /* partition */
        ebx = base;                                 /* 0x8C551 pb */
        pa = base;                                  /* 0x8C55A */
        pd = base + (uintptr_t)((n - 1) * w);       /* 0x8C543..0x8C557; 0x8C55D */
        pc = pd;                                    /* 0x8C560 */
        for (;;) {
            while (ebx <= pc) {                     /* 0x8C563 cmp ebx, [ebp-4]; ja 0x8C5AD */
                r = cmp(PTR(ebx), PTR(piv));        /* 0x8C56D */
                if (r > 0)                          /* 0x8C572 jg 0x8C5AD */
                    break;
                if (r == 0) {                       /* 0x8C574 jne 0x8C5A5 */
                    if (mode != 0)                  /* 0x8C576 */
                        f8C2B3(pa, ebx, w);         /* 0x8C587 esi = pa, edi = pb */
                    else
                        swap4(pa, ebx);             /* 0x8C58E */
                    pa += w;                        /* 0x8C5A2 */
                }
                ebx += w;                           /* 0x8C5A5 */
            }
            while (ebx <= pc) {                     /* 0x8C5AD cmp ebx, [ebp-4]; ja 0x8C602 */
                r = cmp(PTR(pc), PTR(piv));         /* 0x8C5B8 */
                if (r < 0)                          /* 0x8C5BD jl 0x8C602 */
                    break;
                if (r == 0) {                       /* 0x8C5BF jne 0x8C5F7 */
                    if (mode != 0)                  /* 0x8C5C1 */
                        f8C2B3(pc, pd, w);          /* 0x8C5D3 esi = pc, edi = pd */
                    else
                        swap4(pc, pd);              /* 0x8C5DA */
                    pd -= w;                        /* 0x8C5F4 */
                }
                pc -= w;                            /* 0x8C5FD */
            }
            if (ebx > pc)                           /* 0x8C602 ja 0x8C641 */
                break;
            if (mode != 0)                          /* 0x8C607 */
                f8C2B3(ebx, pc, w);                 /* 0x8C618 esi = pb, edi = pc */
            else
                swap4(pc, ebx);                     /* 0x8C61F */
            ebx += w;                               /* 0x8C633 */
            pc -= w;                                /* 0x8C639 */
        }

        /* the equal runs to the middle */
        eend = base + (uintptr_t)(n * w);           /* 0x8C641..0x8C656; 0x8C664 [ebp-0x28] */
        sr = (int32_t)(uint32_t)(ebx - pa);         /* 0x8C654 ecx = pb; 0x8C65B sub ecx, pa */
        st = (int32_t)(uint32_t)(pa - base);        /* 0x8C658, 0x8C65E */
        if (st < sr)                                /* 0x8C667 cmp eax, ecx; jge (signed) */
            sr = st;                                /* 0x8C66B */
        if (sr != 0)                                /* 0x8C66D test; jbe 0x8C6A0 */
            f8C2B3(base, ebx - (uint32_t)sr, (uint32_t)sr);   /* 0x8C671..0x8C69F (inline): esi = base, edi = pb - r */
        ur = (uint32_t)(pd - pc);                   /* 0x8C6A3 ecx = pd; 0x8C6A9 sub ecx, pc */
        ut = (uint32_t)(eend - pd) - w;             /* 0x8C6A0..0x8C6AC */
        if (!(ur < ut))                             /* 0x8C6B2 cmp ecx, eax; jb (unsigned) */
            ur = ut;                                /* 0x8C6B6 */
        if (ur != 0)                                /* 0x8C6B8 test; jbe 0x8C6E8 */
            f8C2B3(ebx, eend - ur, ur);             /* 0x8C6BC..0x8C6E7 (inline): esi = pb, edi = end - r */

        /* push the larger part, go on with the smaller */
        right = (uint32_t)(pd - pc);                /* 0x8C6E8 esi = pd; 0x8C6F6 sub esi, pc */
        ecx = eend - right;                         /* 0x8C6EB, 0x8C6FC */
        left = (uint32_t)(ebx - pa);                /* 0x8C6EE edi = pb; 0x8C6F3 sub edi, pa */
        if (right >= left) {                        /* 0x8C6FE cmp esi, edi; jb 0x8C726 */
            stk_n[sp] = right / w;                  /* 0x8C706 div; 0x8C70E */
            stk_base[sp] = ecx;                     /* 0x8C71D */
            n = left / w;                           /* 0x8C717 div; 0x8C760 */
        } else {
            if (left <= w)                          /* 0x8C726 cmp edi, w; jbe 0x8C415 */
                goto pop;
            stk_base[sp] = base;                    /* 0x8C73A */
            stk_n[sp] = left / w;                   /* 0x8C743 div; 0x8C74B */
            n = right / w;                          /* 0x8C754 div; 0x8C760 */
            base = ecx;                             /* 0x8C75A */
        }
        sp++;                                       /* 0x8C766 */
        continue;                                   /* 0x8C769 jmp 0x8C377 */

pop:
        if (sp == 0)                                /* 0x8C415 cmp 0; je 0x8C328 (the shared epilogue) */
            return;
        sp--;                                       /* 0x8C41F */
        base = stk_base[sp];                        /* 0x8C425 [ebp + 4*sp - 0x144]; 0x8C433 */
        n = stk_n[sp];                              /* 0x8C42C [ebp + 4*sp - 0xC4]; 0x8C439 */
    }
}
