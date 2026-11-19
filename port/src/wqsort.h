#ifndef WQSORT_H
#define WQSORT_H

/* Watcom C/C++32 (1994) qsort, 0x8C331 in AC_MAIN.EXE, translated literally (wqsort.c). Not stable: the order of
   elements that compare equal is exactly the original's, which the AI's choices depend on (docs/M6_PLAN.md, risk 3).
   Never replace it with the C library's qsort.

   The swap mode, like the original's, depends on the address: (base | size) & 3 != 0 or size > 4 swaps the pivot into
   the first element; size == 4 with a 4-aligned base compares against a copy of the pivot. These give different orders
   for ties, so a caller must pass the element block at the same address alignment as the original's (for the data
   object: DP(link address); g_ds is malloc'd, so DP(a) % 4 == a % 4). cmp receives element pointers, or a pointer to
   a 4-byte local copy of the pivot in the size-4 aligned mode. */

#include <stddef.h>

void wc_qsort(void *base, size_t n, size_t size, int (*cmp)(const void *, const void *));

#endif
