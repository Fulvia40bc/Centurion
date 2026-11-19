/* Round-trip check of savewrite.c without the game: for each save file, read it with f5F05C into a fresh data
   object, write it again with f5E9B0 (the description read from the file) into a temp file, append the CRC as
   0x5E8EC does (f7BB84 over the body after its 0x4B header, 4 bytes little-endian), and compare with the original
   file byte for byte.
   Build: clang -std=c11 port/test/save_roundtrip.c port/src/savewrite.c port/src/saveload.c port/src/state.c port/src/le_image.c -Iport/src -o save_roundtrip.exe
   Run:   save_roundtrip GAMEDIR FILE...   (GAMEDIR holds AC_MAIN.EXE for the data object; without it a blank object
          with the version stamp at 0xBC8E4 is used, enough for synthetic saves. If GAMEDIR/aciv.cfg exists, the
          word at 0xF0982 is computed from it as the game's start-up does, main 0x107D7..0x10813: with w7c = the
          word at aciv.cfg+0 and w80 = the word at +4, (w7c >> 9) + (((w80 & 0xFF) >> 5) << 7); the reader f5F05C
          skips those 2 file bytes, so the rewrite writes this value and real saves match. The CRC table at
          0x11260C is built as title.c's f7BB14 does.) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "saveload.h"
#include "savewrite.h"
#include "state.h"

/* ---- Minimal stubs for what saveload.c / savewrite.c link against (as in crc_check.c) ------------------------ */
/* g_ds and g_original come from state.c (linked in for state_init/le_image) */

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

/* f7BB14's table (title.c) at 0x11260C, reflected CRC-32, polynomial 0xEDB88320, and the mode string "rb"
   f7BBF0 passes from the data object (0xB976D) */
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

/* main 0x107D7..0x10813: start-up computes the word at 0xF0982 from aciv.cfg, and the save's 2 skipped bytes at
   0xF0982 hold it. w7c = the word at aciv.cfg+0, w80 = the word at aciv.cfg+4. No aciv.cfg (or a short one):
   the data object's value is left as it is (0 in a blank object). */
static void cfg_word(const char *gamedir)
{
    char path[1024];
    FILE *f;
    uint8_t b[6];

    snprintf(path, sizeof path, "%s/aciv.cfg", gamedir);
    f = fopen(path, "rb");
    if (f == NULL)
        return;
    if (fread(b, 1, sizeof b, f) == sizeof b) {
        uint16_t w7c = (uint16_t)(b[0] | b[1] << 8);
        uint16_t w80 = (uint16_t)(b[4] | b[5] << 8);
        setD16(0xF0982u, (uint16_t)((w7c >> 9) + (uint16_t)(((w80 & 0xFFu) >> 5) << 7)));
    }
    fclose(f);
}

/* A fresh data object per file: the real one from <gamedir>/AC_MAIN.EXE if present, else a blank one with the
   version stamp at 0xBC8E4 (what title.c's "1.11B1" reads) so synthetic saves round-trip too. */
static void fresh_ds(const char *gamedir)
{
    if (g_ds != NULL)
        free(g_ds);
    g_ds = NULL;
    if (state_init(gamedir) != 0) {
        g_ds = calloc(1, DS_SIZE);
        memcpy(DP(0xBC8E4u), "1.11B1", 7);           /* 6 chars + NUL, as the EXE's data object holds it */
    }
    crc_table();
    cfg_word(gamedir);
}

int main(int argc, char **argv)
{
    int a, bad = 0;

    if (argc < 3) {
        fprintf(stderr, "usage: %s GAMEDIR FILE...\n", argv[0]);
        return 2;
    }
    fresh_ds(argv[1]);
    for (a = 2; a < argc; a++) {
        FILE *fp, *out;
        long size, body, got;
        uint8_t *orig, *again;
        int16_t version = 0;
        char desc[0x34], desc2[0x34];
        uint32_t crc;
        long diff = -1;
        long i;

        fp = fopen(argv[a], "rb");
        if (fp == NULL) {
            fprintf(stderr, "%s: cannot read\n", argv[a]);
            return 2;
        }
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        orig = malloc(size > 0 ? (size_t)size : 1);
        if (orig == NULL || fread(orig, 1, (size_t)size, fp) != (size_t)size) {
            fprintf(stderr, "%s: cannot read\n", argv[a]);
            return 2;
        }
        fclose(fp);

        fresh_ds(argv[1]);
        memset(desc, 0, sizeof desc);
        fp = fopen(argv[a], "rb");
        f5F05C(&version, desc, fp);
        fclose(fp);

        memcpy(desc2, desc, sizeof desc2);
        out = tmpfile();
        if (out == NULL) {
            fprintf(stderr, "tmpfile failed\n");
            return 2;
        }
        f5E9B0(desc2, out);
        body = ftell(out);
        /* append the CRC as 0x5E8EC does: f7BBF0 over the body = f7BB84(body - 0x4B, 0, body + 0x4B) for version >= 0x28 */
        {
            uint8_t *bb = malloc((size_t)body);
            uint8_t cb[4];
            fseek(out, 0, SEEK_SET);
            if (fread(bb, 1, (size_t)body, out) != (size_t)body) {
                fprintf(stderr, "tmpfile read failed\n");
                return 2;
            }
            crc = f7BB84((int32_t)(body - 0x4B), 0, bb + 0x4B);
            free(bb);
            cb[0] = (uint8_t)crc;
            cb[1] = (uint8_t)(crc >> 8);
            cb[2] = (uint8_t)(crc >> 16);
            cb[3] = (uint8_t)(crc >> 24);
            (void)fwrite(cb, 1, 4, out);
        }
        got = ftell(out);
        fseek(out, 0, SEEK_SET);
        again = malloc(got > 0 ? (size_t)got : 1);
        if (again == NULL || fread(again, 1, (size_t)got, out) != (size_t)got) {
            fprintf(stderr, "tmpfile read failed\n");
            return 2;
        }
        fclose(out);

        if (got != size) {
            diff = got < size ? got : size;
        } else {
            for (i = 0; i < size; i++)
                if (orig[i] != again[i]) {
                    diff = i;
                    break;
                }
        }
        if (diff < 0)
            printf("%s 0x%X %ld equal\n", argv[a], (unsigned)version, size);
        else {
            printf("%s 0x%X %ld first-difference-offset %ld\n", argv[a], (unsigned)version, size, diff);
            bad = 1;
        }
        free(orig);
        free(again);
    }
    return bad;
}
