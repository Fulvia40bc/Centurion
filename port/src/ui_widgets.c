/* Small UI-kit helpers of AC_MAIN.EXE: the sprite painter 0x3AF8C and the button-by-id helpers 0x3BF30 and 0x3BFBC.
   Literal translations; the address of each instruction a line comes from is in its comment.
   D = read in the code, G = inference. Prototypes and the shared layouts: ui.h. */

#include <stddef.h>
#include <stdlib.h>
#include "ui.h"
#include "fg.h"

/* Sprite pixel blocks (port, G). The original keeps a heap pointer in the sprite record's first dword (+0, filled by
   the capture routine 0x3AE90 from the allocator 0x822B5). A 64-bit pointer does not fit there, so the port keeps the
   blocks in this table and stores a handle in that dword instead: 0 = no block, n = s_spr_heap[n - 1]. Because the
   handle lives in the record itself, a copy of the 8 bytes of a record still draws the same pixels, as a copied
   pointer would in the original. ui_sprite_put is for the translation of 0x3AE90 (not yet written); neither function
   is in ui.h yet. */
#define UI_SPR_MAX 128
static uint8_t *s_spr_heap[UI_SPR_MAX];


int ui_sprite_put(uint32_t spr, uint8_t *pixels)
{
    uint32_t h = D32(spr + UI_SPR_DATA);
    int i;

    if (h >= 1 && h <= UI_SPR_MAX) {                  /* the record has a slot already: reuse it */
        s_spr_heap[h - 1] = pixels;
        return 0;
    }
    for (i = 0; i < UI_SPR_MAX; i++)
        if (s_spr_heap[i] == NULL) {
            s_spr_heap[i] = pixels;
            setD32(spr + UI_SPR_DATA, (uint32_t)(i + 1));
            return 0;
        }
    return -1;
}

uint8_t *ui_sprite_pixels(uint32_t spr)
{
    uint32_t h = D32(spr + UI_SPR_DATA);

    if (h < 1 || h > UI_SPR_MAX)
        return NULL;
    return s_spr_heap[h - 1];
}

/* 0x3AF8C: draw sprite spr with its top-left at (x, y) on the active surface (colour 0 transparent) */
void f3AF8C(uint32_t spr, int x, int y)
{
    const uint8_t *pixels = ui_sprite_pixels(spr);    /* 0x3AFE0 mov ebx,[eax]: the pixel block */

    /* Port guard (G): a record whose block was never captured (the start-up routine 0x118F8, which captures the
       setup screens' boxes, is not translated yet) draws nothing. The original would pass its null pointer on. */
    if (pixels == NULL)
        return;
    fg_move(x, (int)D16(spr + UI_SPR_H) + y - 1);     /* 0x3AFA9..0x3AFBB: word h & 0xFFFF, + y, dec */
    fg_clpimage(pixels,                               /* 0x3AFE3 */
                (int)D16(spr + UI_SPR_W),             /* 0x3AFD3 word [spr+4] & 0xFFFF */
                (int)D16(spr + UI_SPR_H));            /* 0x3AFC6 word [spr+6] & 0xFFFF */
}

/* 0x3BF30: the enabled byte of the button with this id (the first match), or 0 with the "Button not found" box */
int f3BF30(uint32_t dlg, int id)
{
    int32_t i, found = -1;   /* [ebp-0x10], [ebp-0xC] */

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                                  /* 0x3BF5E movsx word [dlg]; jle */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {                   /* 0x3BF7F cmp ax,dx (16 bits) */
            found = i;                                                          /* 0x3BF87, then out of the loop */
            break;
        }
    if (found == -1) {                                                          /* 0x3BF8A */
        f3A7B0(0xB3D02u);                                                       /* 0x3BF95: "Button not found" */
        return 0;                                                               /* 0x3BF9A xor eax,eax */
    }
    return S8(dlg + DLG_REC(found) + BTN_ENABLED);                              /* 0x3BFB0 dword [dlg+20*i+0xF] sar 24 */
}

/* 0x3BFBC: set the enabled byte of the button with this id (the first match) without redrawing it */
void f3BFBC(uint32_t dlg, int id, int enabled)
{
    int32_t i, found = -1;   /* [ebp-0xC], [ebp-0x8] */

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                                  /* 0x3BFEC */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {                   /* 0x3C00D */
            found = i;                                                          /* 0x3C015 */
            break;
        }
    if (found == -1) {                                                          /* 0x3C018 */
        f3A7B0(0xB3D13u);                                                       /* 0x3C023: "Button not found" */
        return;
    }
    setD8(dlg + DLG_REC(found) + BTN_ENABLED, (uint8_t)enabled);                /* 0x3C043 byte [dlg+20*i+0x12] = bl */
}

/* 0x3AE90: cut sprite record spr from the active surface: w x h pixels with the top-left at (x, y). The pixel block
   is fg_getimage's (lower-left at the cursor, bottom row first) passed through fg_unpack, which in mode 25 and for a
   virtual buffer is a plain copy (docs/LIBRARY_CALLS.md 0x8EB0E), so the port reads straight into the block.
   Arguments: eax spr, edx x, ebx y, ecx w, stack h (ret 4). */
void f3AE90(uint32_t spr, int x, int y, int w, int h)
{
    uint8_t *data = malloc((size_t)(w * h > 0 ? w * h : 1));   /* 0x3AEAC..0x3AEBB malloc(w*h) -> [spr] */
    unsigned char key, aux;

    if (data == NULL || ui_sprite_put(spr, data) != 0) {       /* 0x3AECF..0x3AED8 (the second block is the port's) */
        f3A7B0(0xB3BE4u);                                      /* 0x3AEDF "Insufficient memory for sprite.  Press any key." */
        do {                                                   /* 0x3AEE4 getch */
            fg_waitfor(1);
            fg_intkey(&key, &aux);
        } while (key == 0 && aux == 0);
        free(data);
        return;   /* port: the original would go on and write through a null pointer */
    }
    setD16(spr + 4, (uint16_t)w);                              /* 0x3AEEF word [spr+4] = w */
    setD16(spr + 6, (uint16_t)h);                              /* 0x3AEF9 word [spr+6] = h */
    fg_move(x, y + h - 1);                                     /* 0x3AEFD..0x3AF09 */
    fg_getimage(data, w, h);                                   /* 0x3AF1D, then fg_unpack = copy (0x3AF37) */
}
