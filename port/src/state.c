/* Game state = an image of the original's data segment (DS = 1686 at run time).
 *
 * The original keeps all game state in DS and saves it with a block table at DS:5BDE
 * (24 x {u16 address, u16 length}, 1780 bytes in total = the size of every .CSV save).
 * Loading a save scatters the file into those blocks; saving gathers them back.
 * Everything not in a block keeps its initial value from OVL0.OVL (file offset 0xB400 = DS:0000).
 *
 * Logic ported from the EXE works on this image with the original addresses, so a translation
 * can be checked line by line against disasm/EXE_RUNTIME.clean.asm.
 */
#include "state.h"
#include <stdio.h>
#include <string.h>

uint8_t g_ds[0x10000];

#define SAVE_TABLE 0x5BDE   /* VERIFIED: 24 entries, lengths sum to 1780 */
#define SAVE_SIZE  1780

int state_init(const char *ovl0_path) {
    memset(g_ds, 0, sizeof(g_ds));
    FILE *f = fopen(ovl0_path, "rb");
    if (!f) return -1;
    if (fseek(f, 0xB400, SEEK_SET) != 0) { fclose(f); return -1; }
    size_t n = fread(g_ds, 1, sizeof(g_ds), f);
    fclose(f);
    return n > SAVE_TABLE + 24 * 4 ? 0 : -1;
}

/* Walk the save block table; dir 0 = file -> DS, 1 = DS -> file. */
static int walk_blocks(uint8_t *file, int dir) {
    int off = 0;
    for (int e = 0; ; e++) {
        uint16_t addr = W(SAVE_TABLE + 4 * e), len = W(SAVE_TABLE + 4 * e + 2);
        if (len == 0) break;
        if (off + len > SAVE_SIZE) return -1;
        if (dir == 0) memcpy(&g_ds[addr], file + off, len);
        else          memcpy(file + off, &g_ds[addr], len);
        off += len;
    }
    return off == SAVE_SIZE ? 0 : -1;
}

int state_load(const char *path) {
    uint8_t file[SAVE_SIZE];
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    size_t n = fread(file, 1, sizeof(file), f);
    fclose(f);
    if (n != SAVE_SIZE) return -1;
    return walk_blocks(file, 0);
}

int state_save(const char *path) {
    uint8_t file[SAVE_SIZE];
    if (walk_blocks(file, 1) != 0) return -1;
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    size_t n = fwrite(file, 1, sizeof(file), f);
    fclose(f);
    return n == SAVE_SIZE ? 0 : -1;
}
