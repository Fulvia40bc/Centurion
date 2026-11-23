/* SBPRO2.MDI, the Miles AIL 3.03 OPL3 music driver of the s-song set (Erik's MDI.INI), translated literally from
   its bytes (C:\work\advciv_cmp\sound\fm\sbpro2.bin / sbpro2.asm, notes structure.md, midi.md, opl.md, variants.md
   and critic.md, which overrides the others). Each translated statement carries the driver file offset it comes
   from (/ * drv 0x.... * /); the driver runs with CS = DS = ES = its own segment, so a file offset is also the data
   address the code uses. D = read in the code or the data, G = inference.

   STATE (D). The driver's whole segment is modelled as one 64 KB byte image, m[], addressed by the original
   offsets with 16-bit wrap. So every byte of state the original keeps is kept where it keeps it (slots 0x1485..,
   voices 0x1A49, priorities 0x1A5B, the 0x104 shadow 0x140C, the per-channel controller arrays 0x18F9..0x1979,
   the timbre cache 0xD37..0x1401 and its 4096-byte heap at 0x1A83), and the original's out-of-range indexing
   (a key >= 128 into the drum map 0x19B9, vel >> 3 past the 16-byte velocity table, a word read across two slot
   arrays) reads and writes the same neighbouring bytes. The constant tables
   (0x7FD..0xD2E) come from the driver file itself: fmdrv_new copies the whole SBPRO2.MDI image the caller read
   from the game folder into m[] (the port's source holds no bytes of the original), so the header, the DST
   strings, the 0x7A2 descriptor and the GTL handle 0x5A0 = 0xFFFF are the file's, and so is everything an
   out-of-range read reaches (a TIMB count of 0 walks the whole segment, 0x2DCB). The file's data area
   0xD2F..0x2A82 is zero (D-file), and so are the bytes past the file. The controller arrays are zeroed then and
   never reset (critic §2: 0x2FE1 does not touch them).

   ROUTINES (D unless marked):
     0x2A83  set ports: [0xD35] IO, [0xD33] IO+1, [0xD2F] IO+4, [0xD31] IO+5 (the mixer words, never used)
     0x2AA4  operator write (op, reg, val): bank [0xC7F+op], reg + [0xC5B+op]
     0x2ABE  channel write (voice, reg, val): bank [0xCB5+v], reg + [0xCA3+v]
     0x2AD7  the write primitive: out [0xD35]+2*bank, reg; 6 status reads; out +1, val; 8 reads -> write()
     0x2C47  chip reset: 0x105 = 1, 0x104 = 0, shadow [0x140C] = 0, regs 0x001..0x0F5 from [0xA3C+r], regs
             0x101..0x1F5 from [0xA31+r]: 492 writes
     0x2CA6  find (bank<<8|patch) in the 192-slot cache, or 0xFFFF
     0x2D8B  the first TIMB entry (at MIDI_data 0x370, through the far pointers 0x59C -> 0x598) not cached
     0x2DE3  evict the unprotected in-use cache slot with the smallest LRU stamp (ties: the later slot)
     0x2F15  install one timbre (bank, patch, far ptr) into the cache, then rebind every channel's 0x1989
     0x2FE1  state init (cache, heap, channel indices, slots, voice owners, drum map, round-robin pointers)
     0x3085  voice assignment (2-op round robin through [0x1408]; 0x30E1 4-op through [0x140A] and 0xD21)
     0x3127  release a slot's voice (key-off through 0x3184)
     0x3184  the register update: V (0x31A1), the 0x104 shadow (0x31C7), pair 2 of a 4-op voice (0x327E),
             pair 1 (0x3364), the flag-ordered writes 0x3313..0x334C: 0x3409 reg 20 (flag 0x80), 0x346A reg 40
             (0x40), 0x34DE regs 60/80 (0x20), 0x3542 E0 carrier first (0x10), 0x3578 C0 with the pan (0x08),
             0x35C2 A0/B0 with the pitch 0x35FA (0x01)
     0x36F6  voice stealing: priorities once (0x371D..0x373B), then up to N passes (0x3741)
     0x381D  4-op timbre load (dead with EHGM2FM2: every record is 14 bytes)
     0x38F1  2-op timbre load (the pre-split into 0x1561..0x18E5, dirty 0xF9)
     0x39CC  note-off by the received key 0x14E9, honouring CC64
     0x3A2F  note-on (channel-9 drum map, LRU bump, first free of 20 slots, load, assign)
     0x3B1E  sustain-off: note_off(ch, pitch note 0x14D5) for each held slot of the channel
     0x3B54  the message dispatcher (0x3BB1 bend, 0x3BC0 controllers, 0x3BED program, 0x3C53 channel update
             loop, 0x3C7D CC64, 0x3C93 CC6, 0x3C9A CC123, 0x3CC0 CC121)
     0x3E67  INIT_DEV, function 0x305 (0x2A83, 0x2C47, 0x2FE1): fmdrv_new
     0x3EB4  INIT_INS_MGR, function 0x501 (the DOS open of the GTL file): fmdrv_set_bank
     0x3ED8  MIDI_XMIT, function 0x502: fmdrv_xmit (fmdrv_midi enters 0x3B54 directly)
     0x3F54  INSTALL_T_SET, function 0x503 (DOS seek/read of the GTL file): fmdrv_install_timbres
   Not translated: DRV_INIT 0x3D2F / 0x3CE5 (header, DDT and descriptor fill: no runtime state the port reads),
   GET_INFO 0x3DC8, SERVE 0x3DDD, PARSE_ENV 0x3DE4 / 0x1D8, VERIFY_IO 0x3E3B / 0x2B04..0x2C46 (the timer probe at
   IO+8: no effect on the register stream), SHUTDOWN 0x3E92 / 0x2C8E (reset + 0x105 = 0 at game exit, not needed
   for sound), 0x500 / 0x506 (return -1), 0x504 GET_T_STATUS 0x401C / 0x2D5C and 0x505 PROT_UNPROT_T 0x404A /
   0x2CD6 / 0x2D19 (never called by the host; not in the API), the INT 66h dispatcher 0x132, the unused far wrapper
   0x3D02 around 0x3B54, and the empty 0x3D25 / 0x3D2A. The busy flag, cli/sti and the flag-restoring returns
   have no state to keep.

   D/G MARKS THAT MATTER:
     - D: writes go to bank BH, register BL; the 4-op tables 0xCC7..0xD2B and the 0x104 shadow are live code but
       every EHGM2FM2 timbre has size 0x0E, so 0x104 is never written after the reset (D-file).
     - D: 0x3A2F bumps the LRU clock before stamping (0x3A73); 0x2F15 stamps the old value and then bumps (0x2F84).
     - D: a timbre size other than 0x0E / 0x19 returns from note-on with the slot left in use (0x3B0B).
     - D: the steal priorities are unsigned words, computed once per call; both scans give ties to the later slot.
     - D: the DOS calls (int 21h 4200h seek, 3F00h read) are modelled on the image given to fmdrv_set_bank: a read
       copies what is left before the end of the image and leaves the rest of the buffer stale, as DOS does. The
       first seek's carry is not tested in the original (0x3FA0) and the second (0x3FD2) cannot fail for a valid
       handle (G: DOS accepts any 32-bit position with method 0).
     - G: the far pointers' segment words (0x13FD, 0x1435.., 0x59A, 0x59E) hold DRV_CS, a stand-in for CS; only
       0x2D9F tests one (for non-zero), and every dereference uses the offset in this one segment.
     - G: [bp-8] of 0x36F6 (the 4-op victim) is uninitialised in the original when no 4-op-capable voiced slot
       exists; here it starts at 0. Dead code with this bank.
   THE CRITIC'S QUIRKS KEPT ON PURPOSE (critic.md §1, §2 and its smaller notes):
     - CC6 is stored as the bend range with no RPN check (0x3C93); CC100/101 are ignored.
     - Sustain-off (0x3B1E) and CC123 (0x3C9A) pass the slot's pitch note 0x14D5 to note_off, which matches the
       received key 0x14E9: held drum notes whose fixed note differs from the key stay held, and a second,
       still-held note of the same key on that channel is released too.
     - The int16 wraps of the pitch path: imul low word (0x361D), add ah,bl (0x3646), sar by 5 and 4, the octave
       folds by 0xC0; the 8-bit mul/shl/ah of V and the 8-bit div by 0x7F of the level (0x31A1, 0x346A).
     - CC1/CC64/CC112/CC113 compare signed bytes (jge/jl): a value >= 0x80 counts as negative.
     - The 4-op steal reads [slot + 0xCD9] with a slot index (critic #9, 0x37B2).
     - TIMB count 0 walks 65536 entries (0x2DCB dec di from 0); the install loop spins for ever when no cache slot
       can be evicted (every in-use slot protected, or a timbre larger than the heap): 0x2F41 <-> 0x2F63, as the
       original does. Neither happens with the game's data (structure.md §8.6).
     - Note-on of a key already sounding takes a second slot and voice; one note-off releases both.
   Host-side rules the API adds (G, from structure.md §6.6 / §8.3): fmdrv_xmit cuts its buffer into 0x502 calls
   of at most 0x200 bytes at message boundaries, as the host's queue 0x98F30 flushes; fmdrv_install_timbres builds
   the 0x200 bytes the host copies to MIDI_data ("TIMB", u32 BE size, the body; the bytes after the chunk, which the
   host takes from the next XMI chunk, are zero here); fmdrv_set_bank refuses (returns 0, no file open) an image
   whose index has no bank-0xFF terminator inside it, on which 0x3FA2's read loop would spin for ever on the stale
   record. */

#include "fmdrv.h"

#include <stdlib.h>
#include <string.h>

#define DRV_CS 0x1000u                  /* G: the segment word stored in the far pointers (any non-zero value) */
#define DOS_HANDLE 5u                   /* G: the handle DOS gives the GTL file (any value but 0xFFFF) */

/* header and register block */
#define HDR_IO      0x016u   /* IO_PARMS.IO, written by the host before 0x305 */
#define VDI_AX      0x126u   /* the dispatcher's register block: AX BX CX DX */
#define VDI_BX      0x128u
#define VDI_CX      0x12Au
#define SCRATCH     0x03Au   /* header scratch: the 6-byte GTL index record of 0x503 */
#define MIDI_DATA   0x370u   /* DST + 0x100, 512 bytes */
#define P_MIDI      0x598u   /* far pointer -> MIDI_data */
#define PP_MIDI     0x59Cu   /* far pointer -> 0x598 */
#define GTL_HANDLE  0x5A0u
#define RDBUF       0x5A2u   /* 512-byte timbre read buffer */
/* constant tables */
#define T_FREQ      0x7FDu   /* 12 halftones x 16 fine steps, words */
#define T_OCT       0x97Du   /* [96] = n / 12 */
#define T_HALF      0x9DDu   /* [96] = n % 12 */
#define T_RESET0    0xA3Du   /* regs 0x01..0xF5, read as [0xA3C + r] */
#define T_RESET1    0xB32u   /* regs 0x101..0x1F5, read as [0xA31 + r] */
#define T_VEL       0xC27u   /* [16] velocity curve */
#define T_OP1       0xC37u   /* voice -> first operator */
#define T_OP2       0xC49u   /* voice -> second operator */
#define T_OPREG     0xC5Bu   /* operator -> register offset */
#define T_OPBANK    0xC7Fu   /* operator -> bank */
#define T_CHREG     0xCA3u   /* voice -> channel register offset */
#define T_CHBANK    0xCB5u   /* voice -> bank */
#define T_4OPOK     0xCC7u   /* voice can be the first half of a 4-op pair */
#define T_PARTNER   0xCD9u   /* the 4-op partner channel */
#define T_POP1      0xCEBu   /* the partner's operators */
#define T_POP2      0xCFDu
#define T_4OPBIT    0xD0Fu   /* the voice's bit in reg 0x104 */
#define T_4OPV      0xD21u   /* [6] the 4-op voices */
#define T_SCALE1    0xD27u   /* [4] 4-op scale masks, pair 1 / pair 2 */
#define T_SCALE2    0xD2Bu
/* ports */
#define PORT_MIXA   0xD2Fu
#define PORT_MIXD   0xD31u
#define PORT_DATA   0xD33u
#define PORT_ADDR   0xD35u
/* timbre cache */
#define LRU_CLOCK   0xD37u   /* dword */
#define TC_STAMPLO  0xD3Bu   /* [192] words */
#define TC_STAMPHI  0xEBBu   /* [192] words */
#define TC_OFFSET   0x103Bu  /* [192] words: heap offset */
#define TC_BANK     0x11BBu  /* [192] */
#define TC_PATCH    0x127Bu  /* [192] */
#define TC_FLAGS    0x133Bu  /* [192]: 0x80 in use, 0x40 protected */
#define HEAP_PTR    0x13FBu  /* far pointer to the heap */
#define HEAP_CAP    0x13FFu
#define HEAP_USED   0x1401u
#define X1403       0x1403u  /* zeroed by 0x2FE1, no other use found */
#define X1405       0x1405u
#define X1407       0x1407u
#define RR2         0x1408u  /* 2-op round robin */
#define RR4         0x140Au  /* 4-op round robin */
#define SHADOW104   0x140Cu
/* the 20 note slots */
#define SL_TIMB     0x140Du  /* words: timbre offset */
#define SL_TIMBSEG  0x1435u  /* words: its segment */
#define SL_PEND     0x145Du  /* words: types 1/2 note-off pending (dead) */
#define SL_STATE    0x1485u
#define SL_TYPE     0x1499u
#define SL_VOICE    0x14ADu
#define SL_CHAN     0x14C1u
#define SL_NOTE     0x14D5u  /* the pitch note */
#define SL_KEY      0x14E9u  /* the key as received */
#define SL_TRANS    0x14FDu
#define SL_GAIN     0x1511u
#define SL_HELD     0x1525u
#define SL_DIRTY    0x1539u
#define SL_B0       0x154Du
#define SL_KEYON    0x1561u
#define SL_CNT      0x1575u
#define SL_KSL1     0x1589u
#define SL_KSL2     0x159Du
#define SL_AVEKM1   0x15B1u
#define SL_AVEKM2   0x15C5u
#define SL_AD1      0x15D9u
#define SL_AD2      0x15EDu
#define SL_SR1      0x1601u
#define SL_SR2      0x1615u
#define SL_SCALE1   0x1629u
#define SL_WS12     0x163Du  /* words: low op2, high op1 */
#define SL_MULT2    0x1665u  /* words */
#define SL_MULT1    0x168Du
#define SL_FB       0x16B5u
#define SL_PRI      0x16DDu
#define SL_LVL2     0x1705u
#define SL_LVL1     0x172Du
#define SL_T2FREQ   0x1755u  /* words: type-2 frequency (dead) */
#define SL_KSL3     0x177Du
#define SL_KSL4     0x1791u
#define SL_AVEKM3   0x17A5u
#define SL_AVEKM4   0x17B9u
#define SL_AD3      0x17CDu
#define SL_AD4      0x17E1u
#define SL_SR3      0x17F5u
#define SL_SR4      0x1809u
#define SL_WS34     0x181Du  /* words */
#define SL_MULT4    0x1845u
#define SL_MULT3    0x186Du
#define SL_LVL4     0x1895u
#define SL_LVL3     0x18BDu
#define SL_SCALE2   0x18E5u
/* the 16 MIDI channels */
#define CH_RANGE    0x18F9u  /* CC6 */
#define CH_VOL      0x1909u  /* CC7 */
#define CH_PAN      0x1919u  /* CC10 */
#define CH_BENDL    0x1929u
#define CH_BENDM    0x1939u
#define CH_EXPR     0x1949u  /* CC11 */
#define CH_MOD      0x1959u  /* CC1 */
#define CH_SUS      0x1969u  /* CC64 */
#define CH_PROT     0x1979u  /* CC112 */
#define CH_TIDX     0x1989u
#define CH_BANK     0x1999u  /* CC114 */
#define CH_PROG     0x19A9u
#define DRUM_TIDX   0x19B9u  /* [128] */
#define CH_VOICES   0x1A39u
#define V_OWNER     0x1A49u  /* [18] */
#define PRI         0x1A5Bu  /* [20] words */
#define HEAP        0x1A83u  /* 4096 bytes */

struct fmdrv {
    fmdrv_write_fn write;
    void *user;
    uint8_t *gtl;        /* the file behind DOS handle [0x5A0] (fmdrv_set_bank) */
    uint32_t gtl_len;
    uint32_t gtl_pos;    /* its DOS file pointer */
    uint8_t m[0x10000];  /* the driver's segment, CS = DS = ES */
};

#define B(o)      (d->m[(uint16_t)(o)])
#define SETB(o,v) (d->m[(uint16_t)(o)] = (uint8_t)(v))
#define W(o)      rd16(d, (unsigned)(o))
#define SETW(o,v) wr16(d, (unsigned)(o), (unsigned)(v))

static unsigned rd16(const fmdrv *d, unsigned o)
{
    return (unsigned)d->m[(uint16_t)o] | (unsigned)d->m[(uint16_t)(o + 1u)] << 8;
}

static void wr16(fmdrv *d, unsigned o, unsigned v)
{
    d->m[(uint16_t)o] = (uint8_t)v;
    d->m[(uint16_t)(o + 1u)] = (uint8_t)(v >> 8);
}

static int s8(unsigned v)                   /* a byte as the signed compares (jl, jge) see it */
{
    v &= 0xFFu;
    return (v & 0x80u) ? (int)v - 0x100 : (int)v;
}

static int s16(unsigned v)                  /* a word as the signed compares see it */
{
    v &= 0xFFFFu;
    return (v & 0x8000u) ? (int)v - 0x10000 : (int)v;
}

static unsigned sar16(unsigned v, unsigned n)   /* sar on a word */
{
    v &= 0xFFFFu;
    while (n-- > 0)
        v = (v >> 1) | (v & 0x8000u);
    return v;
}

static void seg_copy(fmdrv *d, unsigned di, unsigned si, unsigned cx)   /* rep movsw; adc cx,cx; rep movsb */
{
    while (cx-- > 0) {                      /* forward; equal to the word copy when di < si or the areas are apart */
        SETB(di, B(si));
        di = (di + 1u) & 0xFFFFu;
        si = (si + 1u) & 0xFFFFu;
    }
}

/* ---- DOS (int 21h) on the GTL image ---- */

static void dos_seek(fmdrv *d, uint32_t pos)        /* AX = 4200h, CX:DX = pos */
{
    d->gtl_pos = pos;
}

static void dos_read(fmdrv *d, unsigned dx, unsigned cx)   /* AX = 3F00h, DS:DX = buffer: a short read at EOF */
{
    unsigned i, n = 0;

    if (d->gtl_pos < d->gtl_len)
        n = (d->gtl_len - d->gtl_pos < cx) ? (unsigned)(d->gtl_len - d->gtl_pos) : cx;
    for (i = 0; i < n; i++)
        SETB(dx + i, d->gtl[d->gtl_pos + i]);
    d->gtl_pos += n;
}

/* ---- the chip ---- */

static void f2A83_set_ports(fmdrv *d, unsigned io)
{
    SETW(PORT_MIXA, io + 4u);                                           /* drv 0x2A8D add ax,4; mov [0xD2F] */
    SETW(PORT_MIXD, io + 5u);                                           /* drv 0x2A93 inc ax; mov [0xD31] */
    SETW(PORT_ADDR, io);                                                /* drv 0x2A98 */
    SETW(PORT_DATA, io + 1u);                                           /* drv 0x2A9B inc ax; mov [0xD33] */
}

static void f2AD7_out(fmdrv *d, unsigned bx, unsigned cl)   /* BH = bank, BL = register, CL = value */
{
    unsigned dx = (W(PORT_ADDR) + 2u * ((bx >> 8) & 0xFFu)) & 0xFFFFu;  /* drv 0x2AD7 dx = [0xD35]; 0x2ADB add dl,bh (x2) */
    unsigned bank = (dx - W(PORT_ADDR)) / 2u;
    /* drv 0x2AE7 out dx,bl; 0x2AE8 six status reads; 0x2AF2 out dx+1,cl; 0x2AF4 eight status reads */
    d->write(d->user, (int)(bank << 8 | (bx & 0xFFu)), (int)(cl & 0xFFu));
}

static void f2AA4_op(fmdrv *d, unsigned op, unsigned reg, unsigned val)
{
    unsigned bx = op & 0xFFu;                                           /* drv 0x2AA7 bl = [bp+4]; bh = 0 */
    unsigned ah = B(T_OPBANK + bx);                                     /* drv 0x2AAC */
    unsigned bl = (B(T_OPREG + bx) + reg) & 0xFFu;                      /* drv 0x2AB0, 0x2AB4 add bl,[bp+6] */
    f2AD7_out(d, ah << 8 | bl, val);                                    /* drv 0x2AB7..0x2ABC */
}

static void f2ABE_ch(fmdrv *d, unsigned v, unsigned reg, unsigned val)
{
    unsigned bx = v & 0xFFFFu;                                          /* drv 0x2AC1 bx = [bp+4] */
    unsigned ah = B(T_CHBANK + bx);                                     /* drv 0x2AC4 */
    unsigned bl = (B(T_CHREG + bx) + reg) & 0xFFu;                      /* drv 0x2AC8, 0x2ACC */
    f2AD7_out(d, ah << 8 | bl, val);                                    /* drv 0x2ACF..0x2AD4 */
}

static void f2C47_reset(fmdrv *d)
{
    unsigned bx;

    f2AD7_out(d, 0x105u, 0x01u);                                        /* drv 0x2C4C */
    f2AD7_out(d, 0x104u, 0x00u);                                        /* drv 0x2C54 */
    SETB(SHADOW104, 0);                                                 /* drv 0x2C5C */
    for (bx = 0x001u; bx <= 0x0F5u; bx++)                               /* drv 0x2C61..0x2C70 jbe */
        f2AD7_out(d, bx, B(0xA3Cu + bx));                               /* drv 0x2C64 */
    for (bx = 0x101u; bx <= 0x1F5u; bx++)                               /* drv 0x2C72..0x2C81 jbe */
        f2AD7_out(d, bx, B(0xA31u + bx));                               /* drv 0x2C75 */
}

/* ---- the timbre cache ---- */

static unsigned f2CA6_find(const fmdrv *d, unsigned ax)                 /* ax = bank << 8 | patch */
{
    unsigned si;

    for (si = 0; si < 0xC0u; si++)                                      /* drv 0x2CAC..0x2CCA jb */
        if ((B(TC_FLAGS + si) & 0x80u) != 0                             /* drv 0x2CB2 */
            && B(TC_BANK + si) == ((ax >> 8) & 0xFFu)                   /* drv 0x2CB9 cmp ah */
            && B(TC_PATCH + si) == (ax & 0xFFu))                        /* drv 0x2CBF cmp al */
            return si;                                                  /* drv 0x2CCF */
    return 0xFFFFu;                                                     /* drv 0x2CCC */
}

static unsigned f2D8B_next_missing(fmdrv *d, unsigned arg4, unsigned arg6)
{
    unsigned si, di;

    (void)arg4;                                                         /* [bp+4]: not read */
    si = arg6 & 0xFFFFu;                                                /* drv 0x2D93 */
    if (si == 0xFFFFu)                                                  /* drv 0x2D96 */
        return 0xFFFFu;
    si = W(si + PP_MIDI);                                               /* drv 0x2D9B lds si,[si+0x59C] */
    if (W(si + 2u) == 0)                                                /* drv 0x2D9F segment of the MIDI_data pointer */
        return 0xFFFFu;
    si = W(si);                                                         /* drv 0x2DA5 lds si,[si] */
    if (W(si) != 0x4954u || W(si + 2u) != 0x424Du)                      /* drv 0x2DA7, 0x2DAD "TIMB" */
        return 0xFFFFu;
    si = (si + 8u) & 0xFFFFu;                                           /* drv 0x2DB4 */
    di = W(si);                                                         /* drv 0x2DB7 count */
    do {
        si = (si + 2u) & 0xFFFFu;                                       /* drv 0x2DB9 */
        if (f2CA6_find(d, W(si)) == 0xFFFFu)                            /* drv 0x2DBE..0x2DC9 */
            return W(si);                                               /* drv 0x2DD0 */
        di = (di - 1u) & 0xFFFFu;                                       /* drv 0x2DCB dec di (count 0 wraps) */
    } while (di != 0);                                                  /* drv 0x2DCC */
    return 0xFFFFu;                                                     /* drv 0x2DD4 */
}

static void f3127_release(fmdrv *d, unsigned si);

static void f2DE3_evict(fmdrv *d)
{
    unsigned bp2 = 0xFFFFu, bp4, bp6, ax = 0xFFFFu, bx = 0, dx = 0xFFFFu, si, di, cx;

    for (si = 0; si < 0xC0u; si++, bx += 2u) {                          /* drv 0x2DFB..0x2E2D */
        if ((B(TC_FLAGS + si) & 0x80u) == 0)                            /* drv 0x2DFE */
            continue;
        if ((B(TC_FLAGS + si) & 0x40u) != 0)                            /* drv 0x2E05 protected */
            continue;
        if (W(TC_STAMPHI + bx) > dx)                                    /* drv 0x2E0C ja */
            continue;
        if (W(TC_STAMPHI + bx) == dx && W(TC_STAMPLO + bx) > ax)        /* drv 0x2E12 jb take; 0x2E14 ja */
            continue;
        ax = W(TC_STAMPLO + bx);                                        /* drv 0x2E1A */
        dx = W(TC_STAMPHI + bx);                                        /* drv 0x2E1E */
        bp2 = si;                                                       /* drv 0x2E22 */
    }
    if (bp2 == 0xFFFFu)                                                 /* drv 0x2E2F */
        return;
    ax = W(TC_OFFSET + 2u * bp2);                                       /* drv 0x2E38..0x2E3D */
    bp6 = ax;                                                           /* drv 0x2E41 */
    di = (W(HEAP_PTR) + ax) & 0xFFFFu;                                  /* drv 0x2E44 les di,[0x13FB]; add di,ax */
    si = W(di);                                                         /* drv 0x2E4A size */
    bp4 = si;                                                           /* drv 0x2E4D */
    si = (si + di) & 0xFFFFu;                                           /* drv 0x2E50 */
    cx = (W(HEAP_PTR) + W(HEAP_USED) - si) & 0xFFFFu;                   /* drv 0x2E52..0x2E5A */
    seg_copy(d, di, si, cx);                                            /* drv 0x2E5E..0x2E64 close the gap */
    SETB(TC_FLAGS + bp2, 0);                                            /* drv 0x2E66 */
    SETW(HEAP_USED, W(HEAP_USED) - bp4);                                /* drv 0x2E6E */
    for (di = 0; di < 0x10u; di++) {                                    /* drv 0x2E75..0x2E91 */
        bx = B(CH_TIDX + di);                                           /* drv 0x2E7A (bh = 0) */
        if (bx != 0xFFu && bx == bp2)                                   /* drv 0x2E7E, 0x2E83 */
            SETB(CH_TIDX + di, 0xFFu);                                  /* drv 0x2E88 */
    }
    for (di = 0; di < 0x80u; di++) {                                    /* drv 0x2E93..0x2EA9 */
        bx = B(DRUM_TIDX + di);                                         /* drv 0x2E96 */
        if (bx == bp2)                                                  /* drv 0x2E9A */
            SETB(DRUM_TIDX + di, 0xFFu);                                /* drv 0x2E9F */
    }
    for (bx = 0, di = 0; bx < 0xC0u; bx++, di += 2u) {                  /* drv 0x2EAB..0x2ED0 */
        if ((B(TC_FLAGS + bx) & 0x80u) == 0)                            /* drv 0x2EB1 */
            continue;
        ax = W(TC_OFFSET + di);                                         /* drv 0x2EB8 */
        if (ax > bp6)                                                   /* drv 0x2EBC jbe */
            SETW(TC_OFFSET + di, ax - bp4);                             /* drv 0x2EC1 */
    }
    bp6 = (bp6 + W(HEAP_PTR)) & 0xFFFFu;                                /* drv 0x2ED2 */
    for (si = 0, di = 0; si < 0x14u; si++, di += 2u) {                  /* drv 0x2ED8..0x2F0C */
        if (B(SL_STATE + si) == 0)                                      /* drv 0x2EDE */
            continue;
        ax = W(SL_TIMB + di);                                           /* drv 0x2EE5 */
        if (ax < bp6)                                                   /* drv 0x2EE9 jb */
            continue;
        if (ax == bp6) {                                                /* drv 0x2EEE jne */
            f3127_release(d, si);                                       /* drv 0x2EF1 */
            SETB(SL_STATE + si, 0);                                     /* drv 0x2EF7 */
        } else {
            SETW(SL_TIMB + di, ax - bp4);                               /* drv 0x2EFE */
        }
    }
}

static void f2F15_install(fmdrv *d, unsigned arg4, unsigned bank, unsigned patch, unsigned p_off, unsigned p_seg)
{
    unsigned ax, bx = 0, di, si, cx, dx, bp2;

    (void)arg4;                                                         /* [bp+4]: not read */
    ax = (bank & 0xFFu) << 8 | (patch & 0xFFu);                         /* drv 0x2F21 al = patch, ah = bank */
    bp2 = f2CA6_find(d, ax);                                            /* drv 0x2F28..0x2F2E */
    if (bp2 == 0xFFFFu) {                                               /* drv 0x2F31 */
        if (((p_off | p_seg) & 0xFFFFu) == 0)                           /* drv 0x2F36..0x2F3C */
            return;
        for (;;) {                                                      /* drv 0x2F41 */
            for (di = 0; di < 0xC0u; di++)                              /* drv 0x2F41..0x2F50 */
                if ((B(TC_FLAGS + di) & 0x80u) == 0)                    /* drv 0x2F44 */
                    break;
            if (di < 0xC0u) {
                si = p_off & 0xFFFFu;                                   /* drv 0x2F54 lds si,[bp+0xA] */
                bx = (W(si) + W(HEAP_USED)) & 0xFFFFu;                  /* drv 0x2F57, 0x2F59 */
                if (bx <= W(HEAP_CAP))                                  /* drv 0x2F5D jbe */
                    break;
            }
            f2DE3_evict(d);                                             /* drv 0x2F63; spins if nothing can go */
        }
        ax = W(HEAP_USED);                                              /* drv 0x2F68 xchg [0x1401],bx */
        SETW(HEAP_USED, bx);
        bx = ax;
        bp2 = di;                                                       /* drv 0x2F6C */
        SETB(TC_PATCH + di, patch);                                     /* drv 0x2F75 */
        SETB(TC_BANK + di, bank);                                       /* drv 0x2F79 */
        SETB(TC_FLAGS + di, 0x80u);                                     /* drv 0x2F7D */
        di <<= 1;                                                       /* drv 0x2F82 */
        ax = W(LRU_CLOCK);                                              /* drv 0x2F84 the old clock */
        dx = W(LRU_CLOCK + 2u);                                         /* drv 0x2F87 */
        SETW(LRU_CLOCK, W(LRU_CLOCK) + 1u);                             /* drv 0x2F8B add 1 */
        if (W(LRU_CLOCK) == 0)
            SETW(LRU_CLOCK + 2u, W(LRU_CLOCK + 2u) + 1u);               /* drv 0x2F90 adc 0 */
        SETW(TC_STAMPLO + di, ax);                                      /* drv 0x2F95 */
        SETW(TC_STAMPHI + di, dx);                                      /* drv 0x2F99 */
        SETW(TC_OFFSET + di, bx);                                       /* drv 0x2F9D */
        di = (W(HEAP_PTR) + bx) & 0xFFFFu;                              /* drv 0x2FA1 les di,[0x13FB]; add di,bx */
        cx = W(si);                                                     /* drv 0x2FA7 size, the word included */
        seg_copy(d, di, si, cx);                                        /* drv 0x2FA9..0x2FAF */
    }
    for (di = 0; di < 0x10u; di++)                                      /* drv 0x2FBA..0x2FD1 */
        if (B(CH_PROG + di) == (patch & 0xFFu)                          /* drv 0x2FBD */
            && B(CH_BANK + di) == (bank & 0xFFu))                       /* drv 0x2FC3 */
            SETB(CH_TIDX + di, bp2);                                    /* drv 0x2FC9 mov [di+0x1989],bl */
}

static void f2FE1_state_init(fmdrv *d)
{
    unsigned di;

    SETW(HEAP_PTR, HEAP);                                               /* drv 0x2FE7 */
    SETW(HEAP_PTR + 2u, DRV_CS);                                        /* drv 0x2FED cs */
    SETW(HEAP_CAP, 0x1000u);                                            /* drv 0x2FF1 */
    SETW(HEAP_USED, 0);                                                 /* drv 0x2FF7 */
    SETW(LRU_CLOCK, 0);                                                 /* drv 0x2FFD */
    SETW(LRU_CLOCK + 2u, 0);                                            /* drv 0x3003 */
    for (di = 0; di < 0xC0u; di++)                                      /* drv 0x3009..0x3016 */
        SETB(TC_FLAGS + di, 0);
    for (di = 0; di < 0x10u; di++) {                                    /* drv 0x3018..0x3033 */
        SETB(CH_TIDX + di, 0xFFu);                                      /* drv 0x301B */
        SETB(CH_VOICES + di, 0);                                        /* drv 0x3020 */
        SETB(CH_PROG + di, 0xFFu);                                      /* drv 0x3025 */
        SETB(CH_BANK + di, 0);                                          /* drv 0x302A */
    }
    for (di = 0; di < 0x14u; di++)                                      /* drv 0x3035..0x3041 */
        SETB(SL_STATE + di, 0);
    for (di = 0; di < 0x12u; di++)                                      /* drv 0x3043..0x304F */
        SETB(V_OWNER + di, 0xFFu);
    for (di = 0; di < 0x80u; di++)                                      /* drv 0x3051..0x305B rep stosb */
        SETB(DRUM_TIDX + di, 0xFFu);
    SETW(X1403, 0);                                                     /* drv 0x305D */
    SETW(X1405, 0);                                                     /* drv 0x3063 */
    SETB(X1407, 0);                                                     /* drv 0x3069 */
    SETW(RR2, 0xFFFFu);                                                 /* drv 0x306E */
    SETW(RR4, 0xFFFFu);                                                 /* drv 0x3074 */
}

/* ---- voices ---- */

static void f3184_update(fmdrv *d, unsigned si);
static void f36F6_steal(fmdrv *d);

static void f3085_assign(fmdrv *d, unsigned si)
{
    unsigned dx, bx, di;

    if (B(SL_TYPE + si) == 3) {                                         /* drv 0x308E */
        dx = 0xFFFFu;                                                   /* drv 0x30E1 */
        di = W(RR4);                                                    /* drv 0x30E4 */
        for (;;) {
            dx = (dx + 1u) & 0xFFFFu;                                   /* drv 0x30EA */
            if (dx == 6) {                                              /* drv 0x30EB */
                f36F6_steal(d);                                         /* drv 0x30D9 */
                return;
            }
            di = (di + 1u) & 0xFFFFu;                                   /* drv 0x30F0 */
            if (di == 6)                                                /* drv 0x30F1 */
                di = 0;
            SETW(RR4, di);                                              /* drv 0x30F9 */
            bx = B(T_4OPV + di);                                        /* drv 0x30FD (bh = 0) */
            if (B(V_OWNER + bx) != 0xFFu)                               /* drv 0x3101 */
                continue;
            if (B(V_OWNER + bx + 3u) != 0xFFu)                          /* drv 0x3108 */
                continue;
            break;
        }
        SETB(SL_VOICE + si, bx);                                        /* drv 0x310F */
        di = bx;                                                        /* drv 0x3113 */
        bx = B(SL_CHAN + si);                                           /* drv 0x3115 */
        SETB(CH_VOICES + bx, B(CH_VOICES + bx) + 1u);                   /* drv 0x3119 */
        SETB(V_OWNER + di, bx);                                         /* drv 0x311D */
        SETB(V_OWNER + di + 3u, bx);                                    /* drv 0x3121 */
    } else {
        dx = 0xFFFFu;                                                   /* drv 0x3095 */
        bx = W(RR2);                                                    /* drv 0x3098 */
        for (;;) {
            dx = (dx + 1u) & 0xFFFFu;                                   /* drv 0x309C */
            if (dx == 0x12u) {                                          /* drv 0x309D */
                f36F6_steal(d);                                         /* drv 0x30D9 */
                return;
            }
            bx = (bx + 1u) & 0xFFFFu;                                   /* drv 0x30A2 */
            if (bx == 0x12u)                                            /* drv 0x30A3 */
                bx = 0;
            SETW(RR2, bx);                                              /* drv 0x30AB written on every probe */
            if (B(V_OWNER + bx) == 0xFFu)                               /* drv 0x30AF */
                break;
        }
        SETB(SL_VOICE + si, bx);                                        /* drv 0x30B6 */
        di = bx;                                                        /* drv 0x30BA */
        bx = (bx & 0xFF00u) | B(SL_CHAN + si);                          /* drv 0x30BC mov bl */
        SETB(CH_VOICES + bx, B(CH_VOICES + bx) + 1u);                   /* drv 0x30C0 */
        SETB(V_OWNER + di, bx);                                         /* drv 0x30C4 */
    }
    SETB(SL_DIRTY + si, 0xF9u);                                         /* drv 0x30C8 */
    f3184_update(d, si);                                                /* drv 0x30CE */
}

static void f3127_release(fmdrv *d, unsigned si)
{
    unsigned bx;

    if (B(SL_VOICE + si) == 0xFFu)                                      /* drv 0x3130 */
        return;
    SETB(SL_KEYON + si, B(SL_KEYON + si) & 0xDFu);                      /* drv 0x3137 */
    SETB(SL_DIRTY + si, B(SL_DIRTY + si) | 0x01u);                      /* drv 0x313C */
    f3184_update(d, si);                                                /* drv 0x3142 key-off */
    bx = B(SL_CHAN + si);                                               /* drv 0x3148 bh = 0 */
    SETB(CH_VOICES + bx, B(CH_VOICES + bx) - 1u);                       /* drv 0x314E */
    bx = B(SL_VOICE + si);                                              /* drv 0x3152 */
    if (B(SL_TYPE + si) == 3)                                           /* drv 0x3156 */
        SETB(V_OWNER + bx + 3u, 0xFFu);                                 /* drv 0x315D */
    SETB(V_OWNER + bx, 0xFFu);                                          /* drv 0x3162 */
    SETB(SL_VOICE + si, 0xFFu);                                         /* drv 0x3167 */
    if (B(SL_TYPE + si) == 3 || B(SL_TYPE + si) == 0)                   /* drv 0x316C, 0x3173 */
        SETB(SL_STATE + si, 0);                                         /* drv 0x317A */
}

typedef struct {                /* the locals of 0x3184, by their [bp-n] */
    unsigned bp2, bp4, bp6;     /* voice, its first and second operator */
    unsigned bp16, bp18;        /* MULT words, op 1 / op 2 */
    unsigned bp1a, bp1c;        /* AVEKM & 0xF0 */
    unsigned bp1e, bp20;        /* level words ((~TL & 0x3F) << 10) */
    unsigned bp22, bp24;        /* KSL */
    unsigned bp26, bp28;        /* AD op 1 / op 2 */
    unsigned bp2a, bp2c;        /* SR op 1 / op 2 */
    unsigned bp2e;              /* WS word: low byte op 2, high byte op 1 */
    unsigned bp30;              /* FB word */
    unsigned bp32;              /* CNT */
    unsigned bp14;              /* scale mask */
} f3184_ops;

static void f3184_pair2(const fmdrv *d, unsigned si, f3184_ops *o)     /* the second pair of a 4-op voice */
{
    unsigned bx = (B(SL_VOICE + si) + 3u) & 0xFFFFu;                    /* drv 0x327E..0x3286 */

    o->bp2 = bx;                                                        /* drv 0x3289 */
    o->bp4 = B(T_OP1 + bx);                                             /* drv 0x328C */
    o->bp6 = B(T_OP2 + bx);                                             /* drv 0x3293 */
    o->bp16 = W(SL_MULT3 + 2u * si);                                    /* drv 0x329C */
    o->bp18 = W(SL_MULT4 + 2u * si);                                    /* drv 0x32A3 */
    o->bp1a = B(SL_AVEKM3 + si);                                        /* drv 0x32AA */
    o->bp1c = B(SL_AVEKM4 + si);                                        /* drv 0x32B1 */
    o->bp1e = W(SL_LVL3 + 2u * si);                                     /* drv 0x32B8 */
    o->bp20 = W(SL_LVL4 + 2u * si);                                     /* drv 0x32BF */
    o->bp22 = B(SL_KSL3 + si);                                          /* drv 0x32C6 */
    o->bp24 = B(SL_KSL4 + si);                                          /* drv 0x32CD */
    o->bp26 = B(SL_AD3 + si);                                           /* drv 0x32D4 */
    o->bp28 = B(SL_AD4 + si);                                           /* drv 0x32DB */
    o->bp2a = B(SL_SR3 + si);                                           /* drv 0x32E2 */
    o->bp2c = B(SL_SR4 + si);                                           /* drv 0x32E9 */
    o->bp30 = 0;                                                        /* drv 0x32F0 FB 0 */
    o->bp2e = W(SL_WS34 + 2u * si);                                     /* drv 0x32F5 */
    o->bp32 = B(SL_CNT + si) >> 1;                                      /* drv 0x32FC shr al */
    o->bp14 = B(SL_SCALE2 + si);                                        /* drv 0x3305 */
}

static void f3184_pair1(const fmdrv *d, unsigned si, f3184_ops *o)
{
    unsigned bx = B(SL_VOICE + si);                                     /* drv 0x3364..0x3368 */

    o->bp2 = bx;                                                        /* drv 0x336C */
    o->bp4 = B(T_OP1 + bx);                                             /* drv 0x336F */
    o->bp6 = B(T_OP2 + bx);                                             /* drv 0x3376 */
    o->bp16 = W(SL_MULT1 + 2u * si);                                    /* drv 0x337F */
    o->bp18 = W(SL_MULT2 + 2u * si);                                    /* drv 0x3386 */
    o->bp1a = B(SL_AVEKM1 + si);                                        /* drv 0x338D */
    o->bp1c = B(SL_AVEKM2 + si);                                        /* drv 0x3394 */
    o->bp1e = W(SL_LVL1 + 2u * si);                                     /* drv 0x339B */
    o->bp20 = W(SL_LVL2 + 2u * si);                                     /* drv 0x33A2 */
    o->bp22 = B(SL_KSL1 + si);                                          /* drv 0x33A9 */
    o->bp24 = B(SL_KSL2 + si);                                          /* drv 0x33B0 */
    o->bp26 = B(SL_AD1 + si);                                           /* drv 0x33B7 */
    o->bp28 = B(SL_AD2 + si);                                           /* drv 0x33BE */
    o->bp2a = B(SL_SR1 + si);                                           /* drv 0x33C5 */
    o->bp2c = B(SL_SR2 + si);                                           /* drv 0x33CC */
    o->bp30 = W(SL_FB + 2u * si);                                       /* drv 0x33D3 */
    o->bp2e = W(SL_WS12 + 2u * si);                                     /* drv 0x33DA */
    o->bp32 = B(SL_CNT + si);                                           /* drv 0x33E1 */
    o->bp14 = B(SL_SCALE1 + si);                                        /* drv 0x33E8 */
}

static unsigned f3184_level(unsigned w, unsigned scaled, unsigned vol, unsigned ksl)   /* drv 0x346A..0x3489 */
{
    unsigned bx = (w & 0xFFFFu) >> 2;                                   /* shr bx (x2) */
    unsigned bl = (bx >> 8) & 0xFFu;                                    /* mov bl,bh: 63 - TL */

    if (scaled) {                                                       /* test [bp-0x14], bit */
        unsigned ax = bl * (vol & 0xFFu);                               /* mov al,bh; mul byte [bp-8] */
        bl = (ax / 0x7Fu) & 0xFFu;                                      /* div cl (0x7F): al, at most 126 */
    }
    return ((~bl) & 0x3Fu) | ksl;                                       /* not bl; and 0x3F; or KSL */
}

static void f3184_update(fmdrv *d, unsigned si)
{
    f3184_ops o;
    unsigned di, ax, bx, cx, dx, cl, dl, bl, vol = 0, flags_bp12 = 0, is4_bp10, freq_bpE;

    if (B(SL_VOICE + si) == 0xFFu)                                      /* drv 0x3190 */
        return;
    if ((B(SL_DIRTY + si) & 0x40u) != 0) {                              /* drv 0x319A */
        di = W(SL_CHAN + si) & 0xFu;                                    /* drv 0x31A1 (a word read, & 0xF) */
        ax = B(CH_VOL + di) * B(CH_EXPR + di);                          /* drv 0x31A8 mul byte */
        ax = (ax << 1) & 0xFFFFu;                                       /* drv 0x31B0 shl ax */
        cl = ax >> 8;                                                   /* drv 0x31B2 mov al,ah */
        cl = cl != 0 ? (cl + 1u) & 0xFFu : 0;                           /* drv 0x31B4 cmp al,1; sbb al,-1 */
        ax = cl * B(SL_GAIN + si);                                      /* drv 0x31B8 */
        ax = (ax << 1) & 0xFFFFu;                                       /* drv 0x31BC */
        cl = ax >> 8;                                                   /* drv 0x31BE */
        vol = cl != 0 ? (cl + 1u) & 0xFFu : 0;                          /* drv 0x31C0..0x31C4 [bp-8] */
    }
    is4_bp10 = B(SL_TYPE + si) == 3 ? 1u : 0u;                          /* drv 0x31C7..0x31D4 [bp-0x10] */
    di = W(SL_VOICE + si) & 0xFFu;                                      /* drv 0x31D7 */
    cl = B(SHADOW104);                                                  /* drv 0x31DF */
    dl = B(T_4OPBIT + di);                                              /* drv 0x31E3 */
    if (is4_bp10 != 0) {                                                /* drv 0x31E7 */
        cl |= dl;                                                       /* drv 0x31EB */
        if (cl != B(SHADOW104)) {                                       /* drv 0x31ED */
            SETB(SHADOW104, cl);                                        /* drv 0x31F6 */
            f2AD7_out(d, 0x104u, cl);                                   /* drv 0x31FA */
        }
    } else {
        cl &= ~dl & 0xFFu;                                              /* drv 0x3202 not dl; and cl,dl */
        if (cl != B(SHADOW104)) {                                       /* drv 0x3206 */
            SETB(SHADOW104, cl);                                        /* drv 0x320C */
            f2AD7_out(d, 0x104u, cl);                                   /* drv 0x3210 */
            f2AA4_op(d, B(T_POP1 + di), 0x80u, 0x0Fu);                  /* drv 0x3216..0x3232 */
            f2AA4_op(d, B(T_POP2 + di), 0x80u, 0x0Fu);                  /* drv 0x3235..0x3251 */
            f2ABE_ch(d, B(T_PARTNER + di), 0xB0u, 0x00u);               /* drv 0x3254..0x3272 */
        }
    }
    if (is4_bp10 != 0) {                                                /* drv 0x3275 */
        f3184_pair2(d, si, &o);                                         /* drv 0x327E..0x3309 */
        flags_bp12 = B(SL_DIRTY + si);                                  /* drv 0x330C [bp-0x12] */
    } else {
        f3184_pair1(d, si, &o);                                         /* drv 0x3364..0x33EF */
    }
    for (;;) {
        if ((B(SL_DIRTY + si) & 0x80u) != 0) {                          /* drv 0x3313 -> 0x3409: reg 20 */
            di = W(SL_CHAN + si) & 0xFu;                                /* drv 0x3409 */
            di = s8(B(CH_MOD + di)) >= 0x40 ? 0x40u : 0u;               /* drv 0x3410..0x341B jge */
            ax = (o.bp16 & 0xFFFFu) >> 4;                               /* drv 0x341E shr ax,4 */
            ax = (ax & 0xFF00u) | (ax >> 8);                            /* drv 0x3425 mov al,ah */
            ax = (ax | di) | o.bp1a;                                    /* drv 0x3427 or ax,di; 0x3429 or al,[bp-0x1a] */
            f2AA4_op(d, o.bp4, 0x20u, ax & 0xFFu);                      /* drv 0x342C..0x343A */
            ax = (o.bp18 & 0xFFFFu) >> 4;                               /* drv 0x3440 */
            ax = (ax & 0xFF00u) | (ax >> 8);                            /* drv 0x3447 */
            ax = (ax | di) | o.bp1c;                                    /* drv 0x3449, 0x344B */
            f2AA4_op(d, o.bp6, 0x20u, ax & 0xFFu);                      /* drv 0x344E..0x345C */
            SETB(SL_DIRTY + si, B(SL_DIRTY + si) & 0x7Fu);              /* drv 0x3462 */
        }
        if ((B(SL_DIRTY + si) & 0x40u) != 0) {                          /* drv 0x331D -> 0x346A: reg 40 */
            f2AA4_op(d, o.bp4, 0x40u, f3184_level(o.bp1e, o.bp14 & 1u, vol, o.bp22));   /* drv 0x346A..0x349A */
            f2AA4_op(d, o.bp6, 0x40u, f3184_level(o.bp20, o.bp14 & 2u, vol, o.bp24));   /* drv 0x34A0..0x34D0 */
            SETB(SL_DIRTY + si, B(SL_DIRTY + si) & 0xBFu);              /* drv 0x34D6 */
        }
        if ((B(SL_DIRTY + si) & 0x20u) != 0) {                          /* drv 0x3327 -> 0x34DE: regs 60, 80 */
            f2AA4_op(d, o.bp4, 0x60u, o.bp26);                          /* drv 0x34DE */
            f2AA4_op(d, o.bp6, 0x60u, o.bp28);                          /* drv 0x34F5 */
            f2AA4_op(d, o.bp4, 0x80u, o.bp2a);                          /* drv 0x350C */
            f2AA4_op(d, o.bp6, 0x80u, o.bp2c);                          /* drv 0x3523 */
            SETB(SL_DIRTY + si, B(SL_DIRTY + si) & 0xDFu);              /* drv 0x353A */
        }
        if ((B(SL_DIRTY + si) & 0x10u) != 0) {                          /* drv 0x3331 -> 0x3542: reg E0 */
            f2AA4_op(d, o.bp6, 0xE0u, o.bp2e & 0xFFu);                  /* drv 0x3542 [bp-0x2e]: the second operator first */
            f2AA4_op(d, o.bp4, 0xE0u, (o.bp2e >> 8) & 0xFFu);           /* drv 0x3559 [bp-0x2d] */
            SETB(SL_DIRTY + si, B(SL_DIRTY + si) & 0xEFu);              /* drv 0x3570 */
        }
        if ((B(SL_DIRTY + si) & 0x08u) != 0) {                          /* drv 0x333B -> 0x3578: reg C0 */
            ax = ((o.bp30 & 0xFFFFu) >> 4) & 0x0E00u;                   /* drv 0x3578 shr ax,4; 0x357F and ah,0x0E */
            cl = (o.bp32 & 0x01u) | (ax >> 8) | 0x30u;                  /* drv 0x3582..0x3589 */
            di = W(SL_CHAN + si) & 0xFu;                                /* drv 0x358B */
            bl = B(CH_PAN + di);                                        /* drv 0x3592 */
            if (bl <= 0x1Bu)                                            /* drv 0x3596 jbe */
                cl &= 0xEFu;                                            /* drv 0x35A4 */
            else if (bl >= 0x64u)                                       /* drv 0x359B jb */
                cl &= 0xDFu;                                            /* drv 0x35A0 */
            f2ABE_ch(d, o.bp2, 0xC0u, cl);                              /* drv 0x35A6..0x35B4 */
            SETB(SL_DIRTY + si, B(SL_DIRTY + si) & 0xF7u);              /* drv 0x35BA */
        }
        if ((B(SL_DIRTY + si) & 0x01u) != 0) {                          /* drv 0x3345 -> 0x35C2: A0/B0 */
            if (is4_bp10 != 1) {                                        /* drv 0x35C2 the second pair has no A0/B0 */
                int write_ab = 1;

                freq_bpE = 0;
                if (B(SL_TYPE + si) == 2) {                             /* drv 0x35CB (dead: type 2 is never set) */
                    freq_bpE = W(SL_T2FREQ + 2u * si) >> 6;             /* drv 0x36AB..0x36B5 */
                } else if ((B(SL_KEYON + si) & 0x20u) == 0) {           /* drv 0x35D6 released */
                    f2ABE_ch(d, o.bp2, 0xB0u, B(SL_B0 + si) & 0xDFu);   /* drv 0x35DD..0x35F4 key-off, no A0 */
                    write_ab = 0;
                } else {
                    bx = B(SL_CHAN + si);                               /* drv 0x35FA bh = 0 */
                    ax = (unsigned)B(CH_BENDM + bx) << 7;               /* drv 0x3600..0x3609 */
                    ax = (ax & 0xFF00u) | ((ax | B(CH_BENDL + bx)) & 0xFFu);    /* drv 0x360B or al */
                    ax = (ax - 0x2000u) & 0xFFFFu;                      /* drv 0x360F */
                    ax = sar16(ax, 5);                                  /* drv 0x3612 */
                    cx = B(CH_RANGE + bx);                              /* drv 0x3617 raw CC6, no RPN check */
                    ax = (unsigned)((long)s16(ax) * (long)cx) & 0xFFFFu;    /* drv 0x361D imul cx: the low word */
                    bx = B(SL_NOTE + si);                               /* drv 0x361F */
                    cx = ax;                                            /* drv 0x3625 */
                    bx = (bx + (unsigned)s8(B(SL_TRANS + si))) & 0xFFFFu;   /* drv 0x3627 cbw; add bx,ax */
                    ax = cx;                                            /* drv 0x362E */
                    bx = (bx - 0x18u) & 0xFFFFu;                        /* drv 0x3630 */
                    do
                        bx = (bx + 0x0Cu) & 0xFFFFu;                    /* drv 0x3633 */
                    while (s16(bx) < 0);                                /* drv 0x3636 jl */
                    bx = (bx + 0x0Cu) & 0xFFFFu;                        /* drv 0x363B */
                    do
                        bx = (bx - 0x0Cu) & 0xFFFFu;                    /* drv 0x363E */
                    while (s16(bx) > 0x5F);                             /* drv 0x3641 jg */
                    ax = (ax & 0x00FFu) | ((((ax >> 8) + bx) & 0xFFu) << 8);    /* drv 0x3646 add ah,bl */
                    ax = (ax + 8u) & 0xFFFFu;                           /* drv 0x3648 */
                    ax = sar16(ax, 4);                                  /* drv 0x364B */
                    ax = (ax - 0xC0u) & 0xFFFFu;                        /* drv 0x3650 */
                    do
                        ax = (ax + 0xC0u) & 0xFFFFu;                    /* drv 0x3653 */
                    while (s16(ax) < 0);                                /* drv 0x3656 jl */
                    ax = (ax + 0xC0u) & 0xFFFFu;                        /* drv 0x365B */
                    do
                        ax = (ax - 0xC0u) & 0xFFFFu;                    /* drv 0x365E */
                    while (s16(ax) > 0x5FF);                            /* drv 0x3661 jg */
                    di = ax >> 4;                                       /* drv 0x3666 shr di,4: the semitone */
                    dx = di;                                            /* drv 0x366D */
                    bx = B(T_HALF + di);                                /* drv 0x366F */
                    di = (bx << 5) + ((ax << 1) & 0x1Fu);               /* drv 0x3675..0x3681 halftone*32 + fine*2 */
                    ax = W(T_FREQ + di);                                /* drv 0x3683 */
                    di = dx;                                            /* drv 0x3687 */
                    bl = (B(T_OCT + di) - 1u) & 0xFFu;                  /* drv 0x3689, 0x368D dec bl */
                    if (s16(ax) < 0)                                    /* drv 0x368F or ax,ax; jge */
                        bl = (bl + 1u) & 0xFFu;                         /* drv 0x3693 0xFExx: one block up */
                    if (s8(bl) < 0) {                                   /* drv 0x3695 or bl,bl; jge */
                        bl = (bl + 1u) & 0xFFu;                         /* drv 0x3699 */
                        ax = sar16(ax, 1);                              /* drv 0x369B */
                    }
                    bl = (bl << 2) & 0xFFu;                             /* drv 0x369D shl bl (x2) */
                    ax = (ax & 0x00FFu) | ((((ax >> 8) & 0x03u) | bl) << 8);    /* drv 0x36A1 and ah,3; or ah,bl */
                    freq_bpE = ax;                                      /* drv 0x36A6 [bp-0xe] */
                }
                if (write_ab) {
                    f2ABE_ch(d, o.bp2, 0xA0u, freq_bpE & 0xFFu);        /* drv 0x36B8..0x36C9 */
                    cl = ((freq_bpE >> 8) | B(SL_KEYON + si)) & 0xFFu;  /* drv 0x36CF, 0x36D2 */
                    SETB(SL_B0 + si, cl);                               /* drv 0x36D6 */
                    f2ABE_ch(d, o.bp2, 0xB0u, cl);                      /* drv 0x36DA..0x36E8 */
                }
            }
            SETB(SL_DIRTY + si, B(SL_DIRTY + si) & 0xFEu);              /* drv 0x36EE */
        }
        if (is4_bp10 == 0)                                              /* drv 0x334F */
            return;
        is4_bp10 = 0;                                                   /* drv 0x3358 */
        SETB(SL_DIRTY + si, flags_bp12);                                /* drv 0x335D the flags again for pair 1 */
        f3184_pair1(d, si, &o);                                         /* drv 0x3364 */
    }
}

static void f36F6_steal(fmdrv *d)
{
    unsigned n_bp2 = 0, vm_bp4 = 0, w_bp6 = 0, vm4_bp8 = 0, si, di, ax, bx, cx, dx;

    for (si = 0; si < 0x14u; si++) {                                    /* drv 0x3704..0x373F */
        if (B(SL_STATE + si) == 0)                                      /* drv 0x370D */
            continue;
        n_bp2 = (n_bp2 + 1u) & 0xFFFFu;                                 /* drv 0x3714 */
        di = W(SL_CHAN + si) & 0xFu;                                    /* drv 0x3719 */
        ax = 0xFFFFu;                                                   /* drv 0x3720 */
        if (s8(B(CH_PROT + di)) < 0x40)                                 /* drv 0x3723 jge: CC112 >= 64 keeps 0xFFFF */
            ax = W(SL_PRI + 2u * si);                                   /* drv 0x372A 0x7FFF */
        cx = B(CH_VOICES + di);                                         /* drv 0x372E */
        ax = ax >= cx ? ax - cx : 0;                                    /* drv 0x3734 sub ax,cx; jae; mov ax,0 */
        SETW(PRI + 2u * si, ax);                                        /* drv 0x373B */
    }
    for (;;) {
        ax = 0;                                                         /* drv 0x3741 best waiting */
        dx = 0xFFFFu;                                                   /* drv 0x3744 least voiced */
        cx = 0xFFFFu;                                                   /* drv 0x3747 least 4-op-capable voiced */
        for (si = 0; si < 0x14u; si++) {                                /* drv 0x374A..0x3751 */
            if (B(SL_STATE + si) == 0)                                  /* drv 0x3753 */
                continue;
            di = W(PRI + 2u * si);                                      /* drv 0x375C */
            bx = B(SL_VOICE + si);                                      /* drv 0x3760 (bh = 0) */
            if (bx == 0xFFu) {                                          /* drv 0x3764 no voice: waiting */
                if (di < ax)                                            /* drv 0x3769 jb: ties to the later slot */
                    continue;
                ax = di;                                                /* drv 0x376D */
                w_bp6 = si;                                             /* drv 0x376F */
                continue;
            }
            if (B(T_4OPOK + bx) != 0 && di <= cx) {                     /* drv 0x3774, 0x377B ja */
                cx = di;                                                /* drv 0x377F */
                vm4_bp8 = si;                                           /* drv 0x3781 */
            }
            if (di <= dx) {                                             /* drv 0x3784 ja: ties to the later slot */
                dx = di;                                                /* drv 0x3788 */
                vm_bp4 = si;                                            /* drv 0x378A */
            }
        }
        if (ax < dx)                                                    /* drv 0x378F jae */
            return;
        if (ax == 0)                                                    /* drv 0x3796 */
            return;
        si = vm_bp4;                                                    /* drv 0x379B */
        bx = w_bp6;                                                     /* drv 0x379E */
        if (B(SL_TYPE + bx) == 3) {                                     /* drv 0x37A1 a 4-op note waits */
            si = vm4_bp8;                                               /* drv 0x37A8 */
            if (B(SL_TYPE + si) != 3) {                                 /* drv 0x37AB */
                unsigned al = B(T_PARTNER + si);                        /* drv 0x37B2 [si+0xCD9]: si is a slot */
                for (di = 0; di < 0x14u; di++)                          /* drv 0x37B6..0x37BD */
                    if (B(SL_STATE + di) != 0 && B(SL_VOICE + di) == al) {  /* drv 0x37BF, 0x37C6 */
                        f3127_release(d, di);                           /* drv 0x37CC */
                        break;
                    }
            }
        }
        bx = B(SL_VOICE + si);                                          /* drv 0x37D3 */
        f3127_release(d, si);                                           /* drv 0x37DA the victim is dropped */
        si = w_bp6;                                                     /* drv 0x37E2 */
        SETB(SL_VOICE + si, bx);                                        /* drv 0x37E5 */
        di = bx;                                                        /* drv 0x37E9 */
        bx = (bx & 0xFF00u) | B(SL_CHAN + si);                          /* drv 0x37EB mov bl */
        SETB(CH_VOICES + bx, B(CH_VOICES + bx) + 1u);                   /* drv 0x37EF */
        SETB(V_OWNER + di, bx);                                         /* drv 0x37F3 */
        if (B(SL_TYPE + si) == 3)                                       /* drv 0x37F7 */
            SETB(V_OWNER + di + 3u, bx);                                /* drv 0x37FE */
        SETB(SL_DIRTY + si, 0xF9u);                                     /* drv 0x3802 */
        f3184_update(d, si);                                            /* drv 0x3808 */
        n_bp2 = (n_bp2 - 1u) & 0xFFFFu;                                 /* drv 0x380E */
        if (n_bp2 == 0)                                                 /* drv 0x3811 */
            return;
    }
}

/* ---- timbre loads into a slot ---- */

static void f38F1_load2(fmdrv *d, unsigned si)
{
    unsigned bx = 2u * si, di, ah, al;

    di = W(SL_TIMB + bx);                                               /* drv 0x38FE lds di (offset) */
    SETB(SL_KEYON + si, 0x20u);                                         /* drv 0x3906 */
    SETB(SL_TYPE + si, 0);                                              /* drv 0x390B */
    SETW(SL_PEND + bx, 0xFFFFu);                                        /* drv 0x3910 */
    SETW(SL_PRI + bx, 0x7FFFu);                                         /* drv 0x3916 the priority base */
    ah = B(di + 8u);                                                    /* drv 0x391C T[8] FB/CNT */
    SETB(SL_CNT + si, ah & 0x01u);                                      /* drv 0x3921..0x3926 */
    SETW(SL_FB + bx, (ah << 8) << 4);                                   /* drv 0x392A shl ax,4 */
    ah = B(di + 4u);                                                    /* drv 0x3934 T[4] KSL/TL op 1 */
    SETB(SL_KSL1 + si, ah & 0xC0u);                                     /* drv 0x3937..0x393C */
    SETW(SL_LVL1 + bx, ((~ah & 0x3Fu) << 8) << 2);                      /* drv 0x3940..0x3949 */
    ah = B(di + 0x0Au);                                                 /* drv 0x394F T[10] op 2 */
    SETB(SL_KSL2 + si, ah & 0xC0u);                                     /* drv 0x3952..0x3957 */
    SETW(SL_LVL2 + bx, ((~ah & 0x3Fu) << 8) << 2);                      /* drv 0x395B..0x3964 */
    al = B(di + 3u);                                                    /* drv 0x3968 T[3] AVEKM op 1 */
    SETB(SL_AVEKM1 + si, al & 0xF0u);                                   /* drv 0x396B..0x396F */
    SETW(SL_MULT1 + bx, (al << 8) << 4);                                /* drv 0x3973..0x3979 al = 0; shl ax,4 */
    al = B(di + 9u);                                                    /* drv 0x397D T[9] op 2 */
    SETB(SL_AVEKM2 + si, al & 0xF0u);                                   /* drv 0x3980..0x3984 */
    SETW(SL_MULT2 + bx, (al << 8) << 4);                                /* drv 0x3988..0x398E */
    SETB(SL_AD1 + si, B(di + 5u));                                      /* drv 0x3992 */
    SETB(SL_SR1 + si, B(di + 6u));                                      /* drv 0x3999 */
    SETB(SL_AD2 + si, B(di + 0x0Bu));                                   /* drv 0x39A0 */
    SETB(SL_SR2 + si, B(di + 0x0Cu));                                   /* drv 0x39A7 */
    SETW(SL_WS12 + bx, (unsigned)B(di + 7u) << 8 | B(di + 0x0Du));      /* drv 0x39AE al = T[13], ah = T[7] */
    SETB(SL_SCALE1 + si, B(SL_CNT + si) | 0x02u);                       /* drv 0x39B8..0x39BE CNT | 2 */
    SETB(SL_DIRTY + si, 0xF9u);                                         /* drv 0x39C2 */
}

static void f381D_load4(fmdrv *d, unsigned si)          /* dead with EHGM2FM2 (no 0x19-byte timbre) */
{
    unsigned bx, di, ah, al;

    f38F1_load2(d, si);                                                 /* drv 0x3823 */
    bx = 2u * si;                                                       /* drv 0x382F */
    di = W(SL_TIMB + bx);                                               /* drv 0x3833 */
    SETB(SL_TYPE + si, 3u);                                             /* drv 0x383B */
    al = (B(di + 8u) & 0x80u) >> 6;                                     /* drv 0x3840..0x3848 CNT2 -> bit 1 */
    SETB(SL_CNT + si, B(SL_CNT + si) | al);                             /* drv 0x384A */
    SETB(SL_SCALE1 + si, B(T_SCALE1 + B(SL_CNT + si)));                 /* drv 0x384F..0x3859 */
    SETB(SL_SCALE2 + si, B(T_SCALE2 + B(SL_CNT + si)));                 /* drv 0x385D..0x3861 */
    ah = B(di + 0x0Fu);                                                 /* drv 0x3868 op 3 KSL/TL */
    SETB(SL_KSL3 + si, ah & 0xC0u);                                     /* drv 0x386B..0x3870 */
    SETW(SL_LVL3 + bx, ((~ah & 0x3Fu) << 8) << 2);                      /* drv 0x3874..0x387D */
    ah = B(di + 0x15u);                                                 /* drv 0x3883 op 4 KSL/TL */
    SETB(SL_KSL4 + si, ah & 0xC0u);                                     /* drv 0x3886..0x388B */
    SETW(SL_LVL4 + bx, ((~ah & 0x3Fu) << 8) << 2);                      /* drv 0x388F..0x3898 */
    al = B(di + 0x0Eu);                                                 /* drv 0x389C op 3 AVEKM */
    SETB(SL_AVEKM3 + si, al & 0xF0u);                                   /* drv 0x38A1..0x38A3 */
    SETW(SL_MULT3 + bx, (al << 8) << 4);                                /* drv 0x38A7..0x38AD */
    al = B(di + 0x14u);                                                 /* drv 0x38B1 op 4 AVEKM */
    SETB(SL_AVEKM4 + si, al & 0xF0u);                                   /* drv 0x38B6..0x38B8 */
    SETW(SL_MULT4 + bx, (al << 8) << 4);                                /* drv 0x38BC..0x38C2 */
    SETB(SL_AD3 + si, B(di + 0x10u));                                   /* drv 0x38C6 */
    SETB(SL_SR3 + si, B(di + 0x11u));                                   /* drv 0x38CD */
    SETB(SL_AD4 + si, B(di + 0x16u));                                   /* drv 0x38D4 */
    SETB(SL_SR4 + si, B(di + 0x17u));                                   /* drv 0x38DB */
    SETW(SL_WS34 + bx, (unsigned)B(di + 0x12u) << 8 | B(di + 0x18u));   /* drv 0x38E2..0x38E8; +0x13 is not read */
}

/* ---- notes ---- */

static void f39CC_note_off(fmdrv *d, unsigned ch, unsigned key)
{
    unsigned si, al, bl;

    for (si = 0; si < 0x14u; si++) {                                    /* drv 0x39D2..0x39DF */
        al = key & 0xFFu;                                               /* drv 0x39D5 (reloaded after each hit) */
        bl = ch & 0xFFu;                                                /* drv 0x39D8 */
        if (B(SL_STATE + si) != 1)                                      /* drv 0x39E1 */
            continue;
        if (B(SL_KEY + si) != al)                                       /* drv 0x39E8 the received key */
            continue;
        if (B(SL_CHAN + si) != bl)                                      /* drv 0x39EE */
            continue;
        if (s8(B(CH_SUS + bl)) >= 0x40) {                               /* drv 0x39F6 jge */
            SETB(SL_HELD + si, 1u);                                     /* drv 0x3A23 */
        } else if (B(SL_TYPE + si) == 3 || B(SL_TYPE + si) == 0) {      /* drv 0x39FD, 0x3A04 */
            f3127_release(d, si);                                       /* drv 0x3A0C */
            SETB(SL_STATE + si, 0);                                     /* drv 0x3A12 */
        } else {
            SETW(SL_PEND + 2u * si, 1u);                                /* drv 0x3A19 (dead: types 1/2) */
        }
    }
}

static void f3A2F_note_on(fmdrv *d, unsigned ch, unsigned key, unsigned vel)
{
    unsigned di, si, bx, ax, al, cl, dl;

    di = ch;                                                            /* drv 0x3A35 */
    bx = B(CH_TIDX + di);                                               /* drv 0x3A38 bh = 0 */
    if (di == 9) {                                                      /* drv 0x3A3E */
        si = key;                                                       /* drv 0x3A43 */
        bx = B(DRUM_TIDX + si);                                         /* drv 0x3A46 */
        if ((bx & 0xFFu) == 0xFFu) {                                    /* drv 0x3A4A */
            ax = (key & 0x00FFu) | 0x7F00u;                             /* drv 0x3A4F ah = 0x7F: bank 127, patch key */
            bx = f2CA6_find(d, ax);                                     /* drv 0x3A54..0x3A5B */
            SETB(DRUM_TIDX + si, bx);                                   /* drv 0x3A5D */
        }
    }
    if ((bx & 0xFFu) == 0xFFu)                                          /* drv 0x3A61 no timbre: silent, no slot */
        return;
    bx = (bx << 1) & 0xFFFFu;                                           /* drv 0x3A69 */
    di = (W(HEAP_PTR) + W(TC_OFFSET + bx)) & 0xFFFFu;                   /* drv 0x3A6B lds di,[0x13FB]; 0x3A6F */
    SETW(LRU_CLOCK, W(LRU_CLOCK) + 1u);                                 /* drv 0x3A73 bump first */
    if (W(LRU_CLOCK) == 0)
        SETW(LRU_CLOCK + 2u, W(LRU_CLOCK + 2u) + 1u);                   /* drv 0x3A78 adc */
    SETW(TC_STAMPLO + bx, W(LRU_CLOCK));                                /* drv 0x3A7D, 0x3A84 */
    SETW(TC_STAMPHI + bx, W(LRU_CLOCK + 2u));                           /* drv 0x3A80, 0x3A88 */
    for (si = 0; si < 0x14u; si++)                                      /* drv 0x3A8C..0x3A9A */
        if (B(SL_STATE + si) == 0)                                      /* drv 0x3A8F */
            break;
    if (si == 0x14u)                                                    /* drv 0x3A9C all 20 in use: dropped */
        return;
    SETB(SL_CHAN + si, ch);                                             /* drv 0x3A9E */
    dl = key & 0xFFu;                                                   /* drv 0x3AA5 */
    SETB(SL_KEY + si, dl);                                              /* drv 0x3AA8 */
    al = 0;                                                             /* drv 0x3AAC */
    cl = B(di + 2u);                                                    /* drv 0x3AAE T[2] */
    if (ch != 9) {                                                      /* drv 0x3AB1..0x3AB7 */
        al = cl;                                                        /* drv 0x3AB9 transpose */
        cl = dl;                                                        /* drv 0x3ABB pitch = key */
    }
    SETB(SL_NOTE + si, cl);                                             /* drv 0x3ABD */
    SETB(SL_TRANS + si, al);                                            /* drv 0x3AC1 */
    al = (vel & 0xFFu) >> 3;                                            /* drv 0x3AC5..0x3ACC shr al (x3) */
    SETB(SL_GAIN + si, B(T_VEL + al));                                  /* drv 0x3ACE xlat cs:[0xC27] */
    bx = 2u * si;                                                       /* drv 0x3AD7 */
    SETW(SL_TIMB + bx, di);                                             /* drv 0x3ADB */
    SETW(SL_TIMBSEG + bx, W(HEAP_PTR + 2u));                            /* drv 0x3ADF ds */
    SETB(SL_STATE + si, 1u);                                            /* drv 0x3AE3 */
    SETB(SL_HELD + si, 0);                                              /* drv 0x3AE8 */
    ax = W(di);                                                         /* drv 0x3AED size word */
    if (ax == 0x19u)                                                    /* drv 0x3AEF */
        f381D_load4(d, si);                                             /* drv 0x3B03 */
    else if (ax == 0x0Eu)                                               /* drv 0x3AF4 */
        f38F1_load2(d, si);                                             /* drv 0x3AFA */
    else
        return;                                                         /* drv 0x3B0B the slot stays in use */
    SETB(SL_VOICE + si, 0xFFu);                                         /* drv 0x3B0D */
    f3085_assign(d, si);                                                /* drv 0x3B13 */
}

static void f3B1E_sustain_off(fmdrv *d, unsigned arg_ch, unsigned di)  /* di: the caller's channel register */
{
    unsigned si;

    for (si = 0; si < 0x14u; si++) {                                    /* drv 0x3B24..0x3B4D */
        if (B(SL_STATE + si) == 0)                                      /* drv 0x3B27 */
            continue;
        if (B(SL_CHAN + si) != (arg_ch & 0xFFu))                        /* drv 0x3B2E */
            continue;
        if (B(SL_HELD + si) == 0)                                       /* drv 0x3B37 */
            continue;
        f39CC_note_off(d, di, W(SL_NOTE + si));                         /* drv 0x3B3E the pitch note, not the key */
    }
}

/* ---- the message dispatcher ---- */

static void f3C53_channel_update(fmdrv *d, unsigned di, unsigned al)
{
    unsigned bx = di, si;

    for (si = 0; si < 0x14u; si++) {                                    /* drv 0x3C55..0x3C78 */
        if (B(SL_STATE + si) == 0)                                      /* drv 0x3C58 */
            continue;
        if (B(SL_CHAN + si) != (bx & 0xFFu))                            /* drv 0x3C5F */
            continue;
        SETB(SL_DIRTY + si, B(SL_DIRTY + si) | al);                     /* drv 0x3C65 */
        f3184_update(d, si);                                            /* drv 0x3C6C (no voice: the bits wait) */
    }
}

static void f3B54_message(fmdrv *d, unsigned status, unsigned d1, unsigned d2)
{
    unsigned si, di, ax, cx, bl, al;

    si = d1 & 0xFFu;                                                    /* drv 0x3B5A */
    di = status & 0xFFFFu;                                              /* drv 0x3B61 */
    ax = di & 0xF0u;                                                    /* drv 0x3B64, 0x3B69 */
    di &= 0x0Fu;                                                        /* drv 0x3B66 */
    cx = d2 & 0xFFu;                                                    /* drv 0x3B6C, 0x3B6E */
    if (ax == 0xB0u) {                                                  /* drv 0x3B71 -> 0x3BC0 controllers */
        if (si == 0x72u) {                                              /* drv 0x3BC0 CC114 */
            SETB(CH_BANK + di, cx);                                     /* drv 0x3C04 used at the next program */
        } else if (si == 0x70u) {                                       /* drv 0x3BC5 CC112 */
            SETB(CH_PROT + di, cx);                                     /* drv 0x3C0A */
        } else if (si == 0x71u) {                                       /* drv 0x3BCA CC113 */
            bl = B(CH_TIDX + di);                                       /* drv 0x3BCF */
            if (bl != 0xFFu) {                                          /* drv 0x3BD3 */
                al = B(TC_FLAGS + bl) & 0xBFu;                          /* drv 0x3BDA, 0x3BDE */
                if (s8(cx) >= 0x40)                                     /* drv 0x3BE0 jl */
                    al |= 0x40u;                                        /* drv 0x3BE5 */
                SETB(TC_FLAGS + bl, al);                                /* drv 0x3BE7 */
            }
        } else if (si == 0x01u) {                                       /* drv 0x3C15 CC1 */
            SETB(CH_MOD + di, cx);                                      /* drv 0x3C50 cs:[bx+di] */
            f3C53_channel_update(d, di, 0x80u);                         /* drv 0x3C53 */
        } else if (si == 0x07u) {                                       /* drv 0x3C1F CC7 */
            SETB(CH_VOL + di, cx);                                      /* drv 0x3C50 */
            f3C53_channel_update(d, di, 0x40u);
        } else if (si == 0x0Bu) {                                       /* drv 0x3C27 CC11 */
            SETB(CH_EXPR + di, cx);                                     /* drv 0x3C50 */
            f3C53_channel_update(d, di, 0x40u);
        } else if (si == 0x0Au) {                                       /* drv 0x3C31 CC10 */
            SETB(CH_PAN + di, cx);                                      /* drv 0x3C50 */
            f3C53_channel_update(d, di, 0x08u);
        } else if (si == 0x40u) {                                       /* drv 0x3C36 CC64 */
            SETB(CH_SUS + di, cx);                                      /* drv 0x3C7D */
            if (s8(cx) < 0x40)                                          /* drv 0x3C81 jl */
                f3B1E_sustain_off(d, di, di);                           /* drv 0x3C89 */
        } else if (si == 0x79u) {                                       /* drv 0x3C3B CC121 */
            SETB(CH_SUS + di, 0);                                       /* drv 0x3CC0 */
            f3B1E_sustain_off(d, di, di);                               /* drv 0x3CC5 */
            SETB(CH_MOD + di, 0);                                       /* drv 0x3CCC */
            SETB(CH_EXPR + di, 0x7Fu);                                  /* drv 0x3CD1 */
            SETB(CH_BENDL + di, 0);                                     /* drv 0x3CD6 */
            SETB(CH_BENDM + di, 0x40u);                                 /* drv 0x3CDB */
            f3C53_channel_update(d, di, 0xC1u);                         /* drv 0x3CE0 */
        } else if (si == 0x7Bu) {                                       /* drv 0x3C43 CC123 */
            for (si = 0; si < 0x14u; si++)                              /* drv 0x3C9A..0x3CBB */
                if (B(SL_STATE + si) == 1 && B(SL_CHAN + si) == (di & 0xFFu))   /* drv 0x3C9D, 0x3CA6 */
                    f39CC_note_off(d, di, W(SL_NOTE + si));             /* drv 0x3CAC the pitch note */
        } else if (si == 0x06u) {                                       /* drv 0x3C48 CC6 */
            SETB(CH_RANGE + di, cx);                                    /* drv 0x3C93 any CC6, no RPN check */
        }
        return;                                                         /* drv 0x3C4D others ignored */
    }
    if (ax == 0xC0u) {                                                  /* drv 0x3B76 -> 0x3BED program */
        SETB(CH_PROG + di, si);                                         /* drv 0x3BEF */
        ax = (unsigned)B(CH_BANK + di) << 8 | si;                       /* drv 0x3BF3 ah = bank */
        SETB(CH_TIDX + di, f2CA6_find(d, ax));                          /* drv 0x3BF8..0x3BFE (0xFF if not cached) */
        return;
    }
    if (ax == 0xE0u) {                                                  /* drv 0x3B7B -> 0x3BB1 bend */
        SETB(CH_BENDL + di, si);                                        /* drv 0x3BB3 */
        SETB(CH_BENDM + di, cx);                                        /* drv 0x3BB7 */
        f3C53_channel_update(d, di, 0x01u);                             /* drv 0x3BBB */
        return;
    }
    if (ax == 0x80u) {                                                  /* drv 0x3B80 */
        f39CC_note_off(d, di, si);                                      /* drv 0x3BA4 velocity ignored */
        return;
    }
    if (ax == 0x90u) {                                                  /* drv 0x3B85 */
        if (cx == 0)                                                    /* drv 0x3B94 jcxz */
            f39CC_note_off(d, di, si);                                  /* drv 0x3BA4 */
        else
            f3A2F_note_on(d, di, si, cx);                               /* drv 0x3B96..0x3B99 */
    }
    /* drv 0x3BAC: An, Dn and the rest are ignored */
}

/* ---- the host functions ---- */

static void f3ED8_midi_xmit(fmdrv *d)
{
    unsigned di, si, ax, bx, cx, dx, k, cl, ch;

    di = W(VDI_CX);                                                     /* drv 0x3EDE */
    if (di != 0) {                                                      /* drv 0x3EE2 */
        si = MIDI_DATA;                                                 /* drv 0x3EE7 */
        do {
            ax = 0;                                                     /* drv 0x3EEA */
            bx = 0;                                                     /* drv 0x3EED */
            cx = 0;                                                     /* drv 0x3EF0 */
            ax = B(si);                                                 /* drv 0x3EF3 */
            if (ax == 0xF0u || ax == 0xF7u) {                           /* drv 0x3EF5, 0x3EF9 -> 0x3F29 sysex */
                si = (si + 1u) & 0xFFFFu;                               /* drv 0x3F2D */
                ax = 0;                                                 /* drv 0x3F2E */
                dx = 0;                                                 /* drv 0x3F31 */
                for (;;) {
                    cl = B(si);                                         /* drv 0x3F3F */
                    si = (si + 1u) & 0xFFFFu;                           /* drv 0x3F41 */
                    ch = cl;                                            /* drv 0x3F42 */
                    cl &= 0x7Fu;                                        /* drv 0x3F44 */
                    ax = (ax & 0xFF00u) | ((ax | cl) & 0xFFu);          /* drv 0x3F47 or al,cl */
                    if ((ch & 0x80u) == 0)                              /* drv 0x3F49 js */
                        break;
                    for (k = 0; k < 7; k++) {                           /* drv 0x3F36..0x3F3D shl ax; rcl dx (x7) */
                        dx = ((dx << 1) | (ax >> 15)) & 0xFFFFu;
                        ax = (ax << 1) & 0xFFFFu;
                    }
                }
                si = (si + ax) & 0xFFFFu;                               /* drv 0x3F4F skip the data */
            } else {
                si = (si + 1u) & 0xFFFFu;                               /* drv 0x3EFD */
                bx = B(si);                                             /* drv 0x3EFE */
                si = (si + 1u) & 0xFFFFu;                               /* drv 0x3F00 */
                if ((ax & 0xF0u) != 0xC0u && (ax & 0xF0u) != 0xD0u) {   /* drv 0x3F01..0x3F0E */
                    cx = B(si);                                         /* drv 0x3F10 */
                    si = (si + 1u) & 0xFFFFu;                           /* drv 0x3F12 */
                }
                f3B54_message(d, ax, bx, cx);                           /* drv 0x3F13..0x3F16 */
            }
            di = (di - 1u) & 0xFFFFu;                                   /* drv 0x3F1C */
        } while (di != 0);                                              /* drv 0x3F1D */
    }
    SETW(VDI_AX, 1u);                                                   /* drv 0x3F1F */
}

static void f3F54_install_t_set(fmdrv *d)
{
    unsigned t;

    SETW(PP_MIDI, P_MIDI);                                              /* drv 0x3F55 */
    SETW(PP_MIDI + 2u, DRV_CS);                                         /* drv 0x3F5B */
    SETW(P_MIDI, MIDI_DATA);                                            /* drv 0x3F5F */
    SETW(P_MIDI + 2u, DRV_CS);                                          /* drv 0x3F65 */
    for (;;) {
        t = f2D8B_next_missing(d, 0, 0);                                /* drv 0x3F69..0x3F80 */
        if (t == 0xFFFFu) {                                             /* drv 0x3F83 */
            SETW(VDI_AX, 1u);                                           /* drv 0x4015 all present */
            return;
        }
        SETW(VDI_BX, t);                                                /* drv 0x3F8B [0x128] = bank << 8 | patch */
        if (W(GTL_HANDLE) == 0xFFFFu)                                   /* drv 0x3F8E no file */
            break;
        dos_seek(d, 0);                                                 /* drv 0x3F97 */
        for (;;) {
            dos_read(d, SCRATCH, 6u);                                   /* drv 0x3FA2 one index record */
            if (B(SCRATCH + 1u) == 0xFFu)                               /* drv 0x3FB1 bank 0xFF: the end */
                goto fail;
            if (W(SCRATCH) == W(VDI_BX))                                /* drv 0x3FB8 */
                break;
        }
        dos_seek(d, (uint32_t)W(SCRATCH + 4u) << 16 | W(SCRATCH + 2u)); /* drv 0x3FC1 cx = [0x3E], dx = [0x3C] */
        dos_read(d, RDBUF, 0x200u);                                     /* drv 0x3FD4 */
        f2F15_install(d, 0, B(VDI_BX + 1u), B(VDI_BX), RDBUF, DRV_CS);  /* drv 0x3FE3..0x4005 */
    }
fail:
    SETW(VDI_AX, 0);                                                    /* drv 0x400E */
}

/* ---- the API ---- */

/* the file the port drives: SBPRO2.MDI as shipped (16,541 bytes, the "AIL3MDI" signature, the device name at 0xBA) */
int fmdrv_is_sbpro2(const uint8_t *drv, size_t len)
{
    static const char name[] = "Creative Labs Sound Blaster Pro (new version)";

    return drv != NULL && len == 16541u && memcmp(drv, "AIL3MDI\x1a", 8) == 0
           && memcmp(drv + 0xBA, name, sizeof name) == 0;
}

fmdrv *fmdrv_new(fmdrv_write_fn write, void *user, const uint8_t *drv, size_t len)
{
    fmdrv *d;

    if (!fmdrv_is_sbpro2(drv, len))
        return NULL;
    d = calloc(1, sizeof *d);
    if (d == NULL)
        return NULL;
    d->write = write;
    d->user = user;
    memcpy(d->m, drv, len);                                             /* the driver as DOS loads it: its tables
                                                                           0x7FD..0xD2E, header and strings; the
                                                                           rest of the 64 KB segment is zero */
    SETW(HDR_IO, 0x220u);                                               /* host 0x9A7E4: MDI.INI IO_ADDR 220 */
    f2A83_set_ports(d, W(HDR_IO));                                      /* drv 0x3E67..0x3E7F function 0x305 */
    f2C47_reset(d);                                                     /* drv 0x3E85 */
    f2FE1_state_init(d);                                                /* drv 0x3E88 */
    SETW(VDI_AX, 1u);                                                   /* drv 0x3E8B */
    return d;
}

void fmdrv_free(fmdrv *d)
{
    if (d == NULL)
        return;
    free(d->gtl);
    free(d);
}

int fmdrv_set_bank(fmdrv *d, const uint8_t *gtl, size_t len)       /* function 0x501 (drv 0x3EB4) */
{
    size_t k;
    int terminated = 0;

    free(d->gtl);
    d->gtl = NULL;
    d->gtl_len = 0;
    d->gtl_pos = 0;
    SETW(GTL_HANDLE, 0xFFFFu);                                          /* drv 0x3ECB the open failed */
    if (gtl == NULL || len == 0 || (uint64_t)len > 0xFFFFFFFFu)
        return 0;
    for (k = 0; k + 1 < len; k += 6)                                    /* the index must end inside the image */
        if (gtl[k + 1] == 0xFFu) {
            terminated = 1;
            break;
        }
    if (!terminated)
        return 0;
    d->gtl = malloc(len);
    if (d->gtl == NULL)
        return 0;
    memcpy(d->gtl, gtl, len);
    d->gtl_len = (uint32_t)len;
    SETW(GTL_HANDLE, DOS_HANDLE);                                       /* drv 0x3EC6 */
    return 1;
}

int fmdrv_install_timbres(fmdrv *d, const uint8_t *timb, size_t len)   /* function 0x503 */
{
    uint8_t chunk[0x200];
    size_t n = len;

    memset(chunk, 0, sizeof chunk);                                     /* G: the host copies on into the next chunk */
    memcpy(chunk, "TIMB", 4);
    chunk[4] = (uint8_t)(len >> 24);
    chunk[5] = (uint8_t)(len >> 16);
    chunk[6] = (uint8_t)(len >> 8);
    chunk[7] = (uint8_t)len;
    if (n > sizeof chunk - 8)
        n = sizeof chunk - 8;
    if (timb != NULL && n > 0)
        memcpy(chunk + 8, timb, n);
    memcpy(&d->m[MIDI_DATA], chunk, sizeof chunk);                      /* host 0x9B552..0x9B57F: DST + 0x100 */
    f3F54_install_t_set(d);
    return W(VDI_AX) != 0 ? 1 : 0;
}

int fmdrv_failed_timbre(const fmdrv *d)
{
    return (int)W(VDI_BX);                                              /* [0x128]: the BX the host reads */
}

void fmdrv_midi(fmdrv *d, const uint8_t *msg, int len)
{
    unsigned status, d1 = 0, d2 = 0;

    if (msg == NULL || len <= 0)
        return;
    status = msg[0];
    if (status == 0xF0u || status == 0xF7u)                             /* drv 0x3EF5 the parser skips sysex */
        return;
    if (len > 1)
        d1 = msg[1];                                                    /* drv 0x3EFE */
    if (len > 2 && (status & 0xF0u) != 0xC0u && (status & 0xF0u) != 0xD0u)
        d2 = msg[2];                                                    /* drv 0x3F10 */
    f3B54_message(d, status, d1, d2);                                   /* drv 0x3F16 */
}

static int msg_len(const uint8_t *p, int left)     /* the 0x3ED8 parser's length of one message; 0 = cut off */
{
    int n, k;
    uint32_t v = 0;

    if (p[0] == 0xF0u || p[0] == 0xF7u) {
        for (k = 1; k < left && k <= 5; k++) {
            v = (v << 7) | (p[k] & 0x7Fu);
            if ((p[k] & 0x80u) == 0)
                break;
        }
        if (k >= left || k > 5)
            return 0;
        n = k + 1;
        if (v > (uint32_t)(left - n))
            return 0;
        return n + (int)v;
    }
    n = ((p[0] & 0xF0u) == 0xC0u || (p[0] & 0xF0u) == 0xD0u) ? 2 : 3;
    return n <= left ? n : 0;
}

void fmdrv_xmit(fmdrv *d, const uint8_t *buf, int len)
{
    int pos = 0;

    if (buf == NULL)
        return;
    while (pos < len) {                                                 /* one 0x502 call per batch (host 0x98F30) */
        int start = pos, bytes = 0, count = 0, n, k;

        while (pos < len && (n = msg_len(buf + pos, len - pos)) > 0) {
            if (count > 0 && bytes + n > 0x200)
                break;
            bytes += n;
            count++;
            pos += n;
        }
        if (count == 0)                                                 /* a cut-off message at the end */
            return;
        for (k = 0; k < bytes && k < 0x200; k++)                        /* DST + 0x100; an over-long sysex alone */
            SETB(MIDI_DATA + (unsigned)k, buf[start + k]);
        SETW(VDI_CX, (unsigned)count);                                  /* the host's CX */
        f3ED8_midi_xmit(d);
    }
}
