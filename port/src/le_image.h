#ifndef LE_IMAGE_H
#define LE_IMAGE_H

/* AC_MAIN.EXE's objects as DOS/4GW would load them, built from the original file at run time (the port ships no
   game data). A C port of tools/le_unbind.py: find the game's LE header behind the DOS/4GW stub, assemble an
   object's pages at its link-time base and apply the internal fixups with link-time targets, so a pointer stored
   in the data object reads as the original's link address (data object 0xB0000.., code object 0x10000..). */

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t base;    /* link address of the object's first byte */
    uint32_t size;    /* virtual size; bytes past the file pages are zero (BSS) */
    uint8_t *bytes;   /* malloc'd, size bytes */
} le_object;

/* Object n (1 = code, 2 = data). 0 ok; -1 not an LE program, -2 object missing, -3 unexpected fixup, -4 memory. */
int  le_load_object(const uint8_t *exe, size_t len, int n, le_object *out);
void le_free_object(le_object *o);

#endif
