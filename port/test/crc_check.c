/* Self-check of saveload.c without the game: the CRC check f7BBF0 (through f7BB84) over save files, and a read of
   each file with f5F05C into a blank data object (the file position at the end must be the file size minus the
   4-byte CRC the autosave appends, 0x5E96C..0x5E996).
   Build: clang -std=c11 port/test/crc_check.c port/src/saveload.c -Iport/src -o crc_check.exe
   Run:   crc_check FILE...   (files are only read) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "saveload.h"

/* ---- Minimal stubs for what saveload.c links against ---------------------------------------------------------- */

uint8_t *g_ds;
int g_original;

/* game_rand, as newgame.c's f78AE4 */
int f78AE4(void)
{
    uint32_t seed = D32(GAME_SEED) * 0x15A4E35u + 1u;
    setD32(GAME_SEED, seed);
    return (int)((seed >> 16) & 0x7FFFu);
}

void f76F70(int n, uint8_t *cards)
{
    (void)n;
    (void)cards;
}

/* heap handles: one live block at a time is enough here (lists only with flag 0x01 of 0xE1624) */
#define TAG 0x80000000u
static uint8_t *s_blk[16];
static uint32_t s_len[16];

uint32_t load_heap_alloc(uint32_t size)
{
    uint32_t i;
    if (size == 0 || size > 0xFFFFFu)
        return 0;
    for (i = 0; i < 16; i++)
        if (s_blk[i] == NULL) {
            s_blk[i] = calloc(1, size);
            s_len[i] = size;
            return TAG | (i + 1u) << 20;
        }
    return 0;
}

uint8_t *load_heap_ptr(uint32_t h, uint32_t n)
{
    uint32_t slot = (h & ~TAG) >> 20, off = h & 0xFFFFFu;
    if ((h & TAG) == 0 || slot < 1 || slot > 16 || s_blk[slot - 1] == NULL || off > s_len[slot - 1] ||
        n > s_len[slot - 1] - off)
        return NULL;
    return s_blk[slot - 1] + off;
}

void load_heap_free(uint32_t h)
{
    uint32_t slot = (h & ~TAG) >> 20;
    if ((h & TAG) == 0 || slot < 1 || slot > 16)
        return;
    free(s_blk[slot - 1]);
    s_blk[slot - 1] = NULL;
}

/* f7BB14's table (title.c) at 0x11260C, and the mode string "rb" f7BBF0 passes from the data object (0xB976D;
   the game loads it with AC_MAIN.EXE's data, this test has a blank data object) */
static void crc_table(void)
{
    int32_t i, k;

    memcpy(DP(0xB976Du), "rb", 3);
    for (i = 0; i <= 0xFF; i++) {
        uint32_t c = (uint32_t)i;
        for (k = 8; k > 0; k--)
            c = (c & 1) ? (c >> 1) ^ 0xEDB88320u : c >> 1;
        setD32(0x11260Cu + 4u * (uint32_t)i, c);
    }
}

int main(int argc, char **argv)
{
    int a, bad = 0;

    g_ds = calloc(1, DS_SIZE);
    if (g_ds == NULL)
        return 2;
    crc_table();
    for (a = 1; a < argc; a++) {
        FILE *fp;
        long size;
        int16_t version = 0;
        char desc[0x34];
        uint32_t crc = f7BBF0(argv[a]);

        printf("%s: f7BBF0 = 0x%08lX\n", argv[a], (unsigned long)crc);
        if (crc != 0)
            bad = 1;
        fp = fopen(argv[a], "rb");
        if (fp == NULL)
            continue;
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        memset(desc, 0, sizeof desc);
        memset(g_ds, 0, DS_SIZE);
        crc_table();
        f5F05C(&version, desc, fp);
        printf("  f5F05C: version 0x%X, \"%s\", read %ld of %ld bytes (+4 CRC), seed 0x%08lX\n", (unsigned)version,
               desc, ftell(fp), size, (unsigned long)D32(GAME_SEED));
        if (ftell(fp) + 4 != size)
            bad = 1;
        fclose(fp);
    }
    free(g_ds);
    return bad;
}
