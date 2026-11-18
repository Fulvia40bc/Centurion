/* The UI kit's screen, palette and mouse helpers, Fastgraph/Image's fg_imagebuf, and the quit routine of AC_MAIN.EXE:
   0x3CC14, 0x3CD6C, 0x3CFE4, 0x775F0, 0x78884, 0x788AC, 0x7895C, 0x789A4, 0x8240F, 0x44CAC (+ 0x78508, which 0x44CAC
   and the start-up call). Literal translations; the address of each instruction a line comes from is in its comment.
   D = read in the code, G = inference. Prototypes and the shared layouts: ui.h. */

#include <stdio.h>
#include <stdlib.h>
#include "ui.h"
#include "fg.h"

/* Heap pointers the original keeps in 32-bit data fields cannot live in the data object on a 64-bit host, so the port
   keeps them here (G: the port's convention until ui.h gets one). [0xF522C] = the fg_imagebuf buffer, malloc'd by the
   start-up (0x10FA4, 0xFF00 bytes; title_boot sets this); f3CD6C hands it to f8240F as the original does. The data
   field 0xF522C itself stays 0 in the port. */
uint8_t *ui_heap_F522C = NULL;

void f78508(void);   /* used by the start-up (title.c) too; not in ui.h */

/* 0x8240F: Fastgraph/Image fg_imagebuf(buffer, size): set the buffer the GIF/FLC routines decode through.
   The port's GIF/FLC decoders need no such buffer (G), so a heap buffer's address is not kept anywhere: only the
   size, and the internal buffer's link address, go to Fastgraph/Image's fields in the data object. */
int f8240F(uint8_t *buffer, uint32_t size)
{
    if (size != 0) {                                  /* 0x82413 mov ecx,[ebp+0xC]; jecxz */
        (void)buffer;                                 /* 0x82421 [0xD668A] = buffer, 0x82423 [0xD668E] = ds */
        setD32(0xD6690u, size);                       /* 0x82426 */
    } else {
        setD32(0xD668Au, 0xD6694u);                   /* 0x8242E..0x8243A: the internal 4096-byte buffer at 0xD6694 */
        setD32(0xD6690u, 0x1000u);                    /* 0x8243F */
    }
    return 0;                                         /* 0x82449 xor eax,eax */
}

/* 0x7895C: push the mouse-visibility stack (int8[15] at 0x112A0C, top first); the new top is 0 */
void f7895C(void)
{
    int32_t i;                                        /* [ebp-4] */
    for (i = 0xE; i > 0; i--)                         /* 0x7896D, 0x7897C cmp 0; jle */
        setD8(0x112A0Cu + (uint32_t)i, D8(0x112A0Bu + (uint32_t)i));   /* 0x78985 */
    setD8(0x112A0Cu, 0);                              /* 0x78996 */
}

/* 0x789A4: pop the mouse-visibility stack; returns the new top. Element 14 keeps its value (D). */
int f789A4(void)
{
    int32_t i;                                        /* [ebp-4] */
    for (i = 0; i < 0xE; i++)                         /* 0x789B5, 0x789C2 cmp 0xE; jge */
        setD8(0x112A0Cu + (uint32_t)i, D8(0x112A0Du + (uint32_t)i));   /* 0x789CB */
    return S8(0x112A0Cu);                             /* 0x789DC dword [0x112A09] sar 24 */
}

/* 0x78884: hide the mouse pointer, remembering its state */
void f78884(void)
{
    f7895C();                                         /* 0x78894 */
    fg_mousevis(0);                                   /* 0x7889B */
}

/* 0x788AC: put the pointer back as it was before the matching f78884 */
void f788AC(void)
{
    fg_mousevis(f789A4());                            /* 0x788BC, 0x788C2 */
}

/* 0x775F0: wait until no mouse button is down */
void f775F0(void)
{
    int x, y, b;
    do {
        fg_mousepos(&x, &y, &b);                      /* 0x7760F fg_mousepos(0xE1628, 0xE162C, 0xE1630) */
        setD32(0xE1628u, (uint32_t)x);
        setD32(0xE162Cu, (uint32_t)y);
        setD32(0xE1630u, (uint32_t)b);
    } while (D32(0xE1630u) != 0);                     /* 0x77614 test edx,edx; jne */
}

/* 0x78508: empty the keyboard buffer (fg_intkey until it returns no key; only the key byte is tested, D 0x7852B) */
void f78508(void)
{
    unsigned char key, aux;                           /* [ebp-0x14], [ebp-0x10] */
    do
        fg_intkey(&key, &aux);                        /* 0x78523 */
    while (key != 0);                                 /* 0x7852B cmp byte [ebp-0x14],0; jne */
}

/* 0x3CC14: show the back buffer [0xF52C8] on the whole screen */
void f3CC14(void)
{
    fg_vbopen(S32(0xF52C8u));                         /* 0x3CC2B */
    f78884();                                         /* 0x3CC33 */
    fg_vbpaste(0, 0x27F, 0, 0x1DF, 0, 0x1DF);         /* 0x3CC4D: minx,maxx,miny,maxy, x,y = lower-left */
    f788AC();                                         /* 0x3CC55 */
    fg_vbclose();                                     /* 0x3CC5A */
}

/* 0x3CD6C: load background picture pic (0xC197C[pic]) from ACIV.CT2 into the buffer [0xF52CC], unless it is there */
void f3CD6C(int8_t pic)
{
    if ((int8_t)D8(0xF52D0u) == pic)                  /* 0x3CD83 cmp al,[ebp-0x14]; je */
        return;
    f8240F(ui_heap_F522C, 0xFF00);                    /* 0x3CD99 fg_imagebuf([0xF522C], 0xFF00) */
    fg_vbopen(S32(0xF52CCu));                         /* 0x3CDA8 */
    fg_move(0, 0);                                    /* 0x3CDB4 */
    cat_show(DSTR(D32((uint32_t)(0xC197C + 4 * (int32_t)pic))), 3, S32(0xF40D8u));  /* 0x3CDC3 movsx; 0x3CDD1 */
    fg_vbclose();                                     /* 0x3CDD9 */
    setD8(0xF52D0u, (uint8_t)pic);                    /* 0x3CDE1 */
}

/* 0x3CFE4: if the palette is faded out ([0xF8E40] == 1), fade it in from black to gifname's palette in 64 steps */
void f3CFE4(const char *gifname)
{
    int16_t target[0x300];                            /* [ebp-0x61C]: 64 * the target 6-bit value */
    int16_t acc[0x300];                               /* [ebp-0xC1C]: 64 * the value now shown */
    int32_t i, step;                                  /* [ebp-0x1C], [ebp-0x18] */

    if ((int8_t)D8(0xF8E40u) != 1)                    /* 0x3CFFE dword [0xF8E3D] sar 24; cmp 1; jne */
        return;
    fg_gifpal(gifname, DP(0xF8B40u));                 /* 0x3D018; the result is not checked (D) */
    for (i = 0; i < 0x300; i++)                       /* 0x3D02D */
        target[i] = (int16_t)((int8_t)D8(0xF8B40u + (uint32_t)i) * 64);   /* 0x3D039 movsx ax,byte; shl 6 */
    for (i = 0; i < 0x300; i++)                       /* 0x3D05D memset(0xF5D58, 0, 0x300) (0x7D5D0 = memset, D) */
        setD8(0xF5D58u + (uint32_t)i, 0);
    for (i = 0; i < 0x300; i++)                       /* 0x3D071 memset(acc, 0, 0x600) */
        acc[i] = 0;
    for (step = 0; step < 0x40; step++) {             /* 0x3D076, 0x3D12F */
        for (i = 0; i < 0x300; i++) {                 /* 0x3D090 */
            int8_t v;
            if (acc[i] >= target[i])                  /* 0x3D0B3 cmp ax,bx; jge (signed 16-bit) */
                continue;
            acc[i] = (int16_t)(acc[i] + (int16_t)((int8_t)D8(0xF8B40u + (uint32_t)i) + 1));  /* 0x3D0BE..0x3D0C7 */
            v = (int8_t)(acc[i] / 64);                /* 0x3D0D2..0x3D0E9: sar 16, signed /64 (truncates), movsx al */
            setD8(0xF5D58u + (uint32_t)i, (uint8_t)(v - 1));        /* 0x3D0EC lea edx,[eax-1]; 0x3D0F2 */
            if ((int8_t)D8(0xF5D58u + (uint32_t)i) < 0)             /* 0x3D0FB sar 24; jge */
                setD8(0xF5D58u + (uint32_t)i, 0);                   /* 0x3D10D */
        }
        /* The original's pace comes from fg_setdacs waiting for the vertical retrace (0x879EF, 70 Hz): 64 steps take
           about 0.9 s on the VGA (G). The port's fg_setdacs has to pace it the same way. */
        fg_setdacs(0, 0x100, DP(0xF5D58u));           /* 0x3D124 */
    }
    fg_setdacs(0, 0x100, DP(0xF8B40u));               /* 0x3D14E: the exact target */
    setD8(0xF8E40u, 0);                               /* 0x3D158 */
}

/* 0x44CAC: quit to DOS: save the settings if they changed, shut down, print [0xF5500], exit(0) */
_Noreturn void f44CAC(void)
{
    if (S32(0xF40E8u) == 1) {                         /* 0x44CBF */
        FILE *f = fopen(DSTR(0xB458Du), DSTR(0xB458Au));   /* 0x44CD2 fopen("aciv.cfg", "wb") (0x7D8C8 = fopen) */
        if (f != NULL) {                              /* 0x44CDD */
            fwrite(DP(0xF097Cu), 6, 1, f);            /* 0x44CF3 fwrite(0xF097C, 6, 1, f) (0x912D5 = fwrite) */
            fclose(f);                                /* 0x44CFB (0x7D95E = fclose) */
        }
    }
    if (D8(0xF625Au) != 0)                            /* 0x44D00: the mouse was initialised */
        fg_mousefin();                                /* 0x44D09 */
    /* 0x44D10 fg_setvpage(0): makes page 0 visible; the port has one page (LIBRARY_CALLS: no-op). */
    /* 0x44D1F fg_setmode([0xF6058]): back to the DOS video mode; the port's window closes when the process ends. */
    printf(DSTR(0xB4596u), DSTR(0xF5500u));           /* 0x44D31 printf("%s\n", 0xF5500) (0x7D555 = printf) */
    /* 0x44D43 AIL_shutdown() if [0xF625B]: the port's own sound stops when the process ends. */
    f78508();                                         /* 0x44D48 */
    exit(0);                                          /* 0x44D4F (0x7D578 = exit) */
}
