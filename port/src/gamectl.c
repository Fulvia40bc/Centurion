/* Game Control (EXE segment 0823), Claude 2026-11-09: the menu 0x1A8B (record DS:05A6) with Save 0x762D, Load 0x76C2,
 * New Game 0x752E, About (inline 0x1AFB), Difficulty (inline 0x1B32), Fine Tuning 0x19BC and Quit (inline 0x1BC9).
 * docs/GAMELOOP_NOTES.md section 3 and its verifier corrections; translated from disasm (exe_lin listing of
 * disasm/EXE_RUNTIME.bin) on g_ds with the original addresses.
 *
 * Buffers in DS, as in the original:  AC60 pack buffer (1780 bytes; also the About / disk-prompt text), A87A the file
 * list (9 x 13-byte names), A6C0 the slot descriptions (9 x 49 bytes), A8F0 the description (48 bytes, saved block 1),
 * A922 the 4 header bytes read from each file, A64C the Fine Tuning sub-menu title, 5B12 the file name "s.csv".
 * Save files are read and written in the current folder, as the original does on its current drive.
 *
 * Not ported: the mouse cursor (0x1D55 push / 0x1D8A pop / 0x1CCE show / 0x1D19 hide: no cursor sprite in the port) and
 * the DOS drive switch of Save / Load when DS:0698 != 0 (0x0373 get drive, 0x0363 set drive B(C0E4): two floppies).
 * The disk prompts 0x7B1E are ported (they only show when DS:0698 != 0, 0 in every snapshot).
 * The title / editor colours [6348] / [6346] are the start-up's VGA values (0x0146..0x0152), which the port writes in
 * map.c map_startup before the map loop; a DS straight from OVL0 / a .CSV has 7 / 7 there. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "screen.h"
#include "state.h"
#include "assets.h"
#include "compat.h"

#define PACK_BUF  0xAC60   /* 0x75A3 / 0x75E8 / 0x752E / About text */
#define SAVE_TAB  0x5BE0   /* 0x75A3 walks dx = 5BE0 .. < 5C44 step 4: {W(dx-2) address, W(dx) length} = table DS:5BDE */
#define SAVE_END  0x5C44
#define FILE_LIST 0xA87A   /* 13-byte names, up to 9 (A87A .. A8EF) */
#define SLOT_DESC 0xA6C0   /* 49-byte descriptions, 9 slots (A6C0 .. A879) */
#define DESC_BUF  0xA8F0   /* the description being entered (0x30 bytes) */
#define HDR_BUF   0xA922   /* the 4 bytes compared with "10A" */
#define FILE_NAME 0x5B12   /* "0.csv": byte 0 = '0' + slot */

static char *dsp(uint16_t a) { return (char *)&g_ds[a]; }

/* strcpy / strcat / strcmp / strlen on DS (0x9E32 / 0x9DF2 / 0xA186 / 0xA02C), 16-bit offsets */
static void ds_strcpy(uint16_t d, uint16_t s) {
    for (;; d++, s++) { uint8_t c = g_ds[s]; g_ds[d] = c; if (!c) break; }
}
static void ds_strcat(uint16_t d, uint16_t s) {
    while (g_ds[d]) d++;
    ds_strcpy(d, s);
}
static int ds_strcmp(uint16_t a, uint16_t b) {
    for (;; a++, b++) {
        int d = (int)g_ds[a] - (int)g_ds[b];
        if (d || !g_ds[a]) return d;
    }
}
static int ds_strlen(uint16_t a) { int n = 0; while (g_ds[(uint16_t)(a + n)]) n++; return n; }

static const ui_box_t FULL = { 0, 0, 319, 199 };

/* 0x8BE4 centred text with the colour word [9B69] = attr. 0x8D56 puts (attr >> 8) & 0x3F into the glyph sprite (+9) and
   the VGA blit 0x907B maps the ink nibble through the colour table DS:60C0 + 16 * that (OVL0 file 0x114C0); ui.c's
   ui_text_block_attr uses the nibble as it is, which is table 0 (the identity). A transparent word with a table (here
   only [6348] = 010C: table 1, nibble C -> 26, the box fill, which erases the title) is mapped here. */
static void text8be4(uint8_t *fb, int cx, int y, const char *s, uint16_t attr) {
    if ((attr & 0x8000) || ((attr >> 8) & 0x3F) == 0) { ui_text_block_attr(fb, cx, y, s, attr); return; }
    uint8_t ink = B(0x60C0 + 16 * ((attr >> 8) & 0x3F) + (attr & 0x0F));
    char line[256];
    for (const char *p = s; p; ) {
        const char *nl = strchr(p, '\n');
        size_t n = nl ? (size_t)(nl - p) : strlen(p);
        if (n >= sizeof(line)) n = sizeof(line) - 1;
        memcpy(line, p, n); line[n] = 0;
        ui_text(fb, cx - (int)((unsigned)ui_text_advance(line) >> 1), y, line, ink);
        y += 11;                                                      /* [9B5C] + [9B5D] */
        p = nl ? nl + 1 : NULL;
    }
}

/* ---- the save image ---- */
/* 0x75A3: gather the blocks of the table DS:5BDE into DS:AC60; returns the total (1780) */
static int pack75a3(void) {
    unsigned off = 0;
    for (uint16_t dx = SAVE_TAB; dx < SAVE_END; dx += 4) {
        uint16_t src = W(dx - 2), n = W(dx);
        for (unsigned i = 0; i < n; i++) g_ds[(uint16_t)(PACK_BUF + off + i)] = g_ds[(uint16_t)(src + i)];
        off += n;
    }
    return (int)off;
}
/* 0x75E8: scatter DS:AC60 back into the blocks; returns the total */
static int unpack75e8(void) {
    unsigned off = 0;
    for (uint16_t dx = SAVE_TAB; dx < SAVE_END; dx += 4) {
        uint16_t dst = W(dx - 2), n = W(dx);
        for (unsigned i = 0; i < n; i++) g_ds[(uint16_t)(dst + i)] = g_ds[(uint16_t)(PACK_BUF + off + i)];
        off += n;
    }
    return (int)off;
}

/* DOS file calls on DS buffers (0x9EBE _dos_read / 0x9EC5 _dos_write): 0 = ok */
static int dos_read(FILE *f, uint16_t buf, unsigned n) {
    uint8_t tmp[0x800];
    if (n > sizeof(tmp)) n = sizeof(tmp);
    size_t got = fread(tmp, 1, n, f);
    for (size_t i = 0; i < got; i++) g_ds[(uint16_t)(buf + i)] = tmp[i];
    return ferror(f) ? 5 : 0;              /* end of file is not an error: fewer bytes, AX = 0 */
}
/* 0x9EC5 (INT 21h/40h, then A4DA): an error only when DOS sets CF; a short count (disk full) is not one, the count goes
   to the caller's word and nobody looks at it. So only a stream error other than "no space" counts here. */
static int dos_write(FILE *f, uint16_t buf, unsigned n) {
    uint8_t tmp[0x800];
    if (n > sizeof(tmp)) n = sizeof(tmp);
    for (unsigned i = 0; i < n; i++) tmp[i] = g_ds[(uint16_t)(buf + i)];
    errno = 0;
    (void)fwrite(tmp, 1, n, f);
    return (ferror(f) && errno != ENOSPC) ? 5 : 0;
}

/* 0x752E new_game: the start state from member NEWGAME.LZW (DS:5B02 "newgame.lzw") of archive 3 (CENT):
     n = 75A3(); if (!F7B(3, &a)) return; buf = 7488(n + 1, 1); ok = F0C("newgame.lzw", &a, [C0C8], buf) != 0;
     11A9(&a); if (ok) { memcpy(AC60, buf, n); 75E8(); } 74B7(buf);
   Returns 0 when the state was replaced, -1 when the archive or the member could not be read (DS unchanged apart
   from the pack buffer, as in the original). */
int new_game(void) {
    int n = pack75a3();
    if (assets_load("CENT.DIR", "CENT.DAT") != 0) return -1;            /* 0xF7B(3): archive 3 = cent */
    int k = assets_find(dsp(0x5B02));
    size_t len = 0;
    uint8_t *buf = k >= 0 ? assets_decode(k, &len) : NULL;              /* 0xF0C: u32 size + LZSS */
    if (!buf) return -1;
    for (int i = 0; i < n; i++) g_ds[(uint16_t)(PACK_BUF + i)] = (size_t)i < len ? buf[i] : 0;
    unpack75e8();
    free(buf);
    return 0;
}

/* ---- the slot screens ---- */
/* 0x7B1E disk_prompt(str): only with two floppy drives (DS:0698 != 0): str + "A:" with the drive letter B(C0E4) + 'A' */
static void disk_prompt7b1e(uint16_t str) {
    if (W(0x698) == 0) return;
    ds_strcpy(PACK_BUF, str);
    B(0x5C4E) = (uint8_t)(B(0xC0E4) + 'A');
    ds_strcat(PACK_BUF, 0x5C4E);
    message_box(dsp(PACK_BUF));
}

/* The host folder seen as a DOS drive (as DOSBox mounts it): every host name has an 8.3 name, upper case; a name that
   does not fit 8.3 gets an alias BASE6~N.EXT (spaces and dots dropped, characters DOS does not allow -> '_', the
   extension = the first 3 of the last one). The aliases of the current list are remembered with their host names so
   that 0x7A54 can open them, as DOS opens a file by its 8.3 name. */
static char g_alias[10][13], g_host[10][260];
static int  g_nalias;

static int dos83_char(unsigned char c) {
    if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c >= 0x80) return 1;
    return c && strchr("!#$%&'()-@^_`{}~", c) != NULL;
}
static unsigned char dos_upper(unsigned char c) { return (c >= 'a' && c <= 'z') ? (unsigned char)(c - 0x20) : c; }

/* the 8.3 name of host name `name` into out13; n = the alias number; returns 1 when the name had to be aliased */
static int dos83_name(const char *name, int n, char *out13) {
    const char *ld = strrchr(name, '.');
    if (ld == name) ld = NULL;                                       /* ".x": no base, all of it is the base */
    size_t bl = ld ? (size_t)(ld - name) : strlen(name), el = ld ? strlen(ld + 1) : 0;
    int fits = bl >= 1 && bl <= 8 && el <= 3;
    for (size_t i = 0; fits && i < bl; i++) if (!dos83_char(dos_upper((unsigned char)name[i]))) fits = 0;
    for (size_t i = 0; fits && i < el; i++) if (!dos83_char(dos_upper((unsigned char)ld[1 + i]))) fits = 0;
    int o = 0;
    if (fits) {
        for (size_t i = 0; name[i] && o < 12; i++) out13[o++] = (char)dos_upper((unsigned char)name[i]);
        out13[o] = 0;
        return 0;
    }
    for (size_t i = 0; i < bl && o < 6; i++) {
        unsigned char c = dos_upper((unsigned char)name[i]);
        if (c == ' ' || c == '.') continue;
        out13[o++] = (char)(dos83_char(c) ? c : '_');
    }
    if (o == 0) out13[o++] = '_';
    o += snprintf(out13 + o, 13 - (size_t)o, "~%d", n);
    if (ld) {
        int e = 0;
        for (size_t i = 0; i < el && e < 3 && o < 12; i++) {
            unsigned char c = dos_upper((unsigned char)ld[1 + i]);
            if (c == ' ' || c == '.') continue;
            if (!e) out13[o++] = '.';
            out13[o++] = (char)(dos83_char(c) ? c : '_');
            e++;
        }
    }
    out13[o] = 0;
    return 1;
}

/* DOS "*.csv" (_dos_findfirst attribute 0): plain files, not hidden or system, whose 8.3 name has the extension CSV */
static int dos_match_csv(const char *name, char *out13) {
    if (!strcmp(name, ".") || !strcmp(name, "..")) return 0;
    struct stat st;
    if (stat(name, &st) != 0 || !S_ISREG(st.st_mode)) return 0;
#ifdef _WIN32
    DWORD a = GetFileAttributesA(name);
    if (a != INVALID_FILE_ATTRIBUTES && (a & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))) return 0;
#endif
    int n = 1;
    for (;;) {
        int aliased = dos83_name(name, n, out13);
        const char *dot = strrchr(out13, '.');
        if (!dot || strcmp(dot + 1, "CSV") != 0) return 0;
        if (!aliased) return 1;
        int used = 0;                                                /* ~N not yet taken in this list */
        for (int i = 0; i < g_nalias; i++) if (!strcmp(g_alias[i], out13)) used = 1;
        if (!used || n >= 9) return 1;
        n++;
    }
}
static void alias_add(const char *alias, const char *host) {
    if (g_nalias >= 10) return;
    snprintf(g_alias[g_nalias], sizeof(g_alias[0]), "%s", alias);
    snprintf(g_host[g_nalias], sizeof(g_host[0]), "%s", host);
    g_nalias++;
}
/* the host file behind an 8.3 name of the list (the name itself when it is not an alias) */
static const char *host_name(const char *name83) {
    for (int i = 0; i < g_nalias; i++) if (!strcmp(g_alias[i], name83)) return g_host[i];
    return name83;
}

/* 0x7996 list_files(pattern): the first 9 matches into DS:A87A (13 bytes each), sorted with strcmp; returns the index
   of the last name (0 also when nothing matched: then A87A keeps whatever it held). Pattern DS:5CD4 / 5CE4 "*.csv". */
static int list7996(void) {
    int di = 0;
    DIR *d = opendir(".");
    struct dirent *e;
    char name[13];
    int found = 0;
    uint16_t si = FILE_LIST;
    g_nalias = 0;
    while (d && (e = readdir(d)) != NULL) {
        if (!dos_match_csv(e->d_name, name)) continue;
        alias_add(name, e->d_name);
        if (!found) { found = 1; }                                   /* findfirst: strcpy(A87A, name) */
        else {                                                       /* findnext */
            di++; si += 13;
            if (si >= 0xA8EF) { di = 8; break; }
        }
        for (int i = 0; i < 13; i++) { g_ds[(uint16_t)(si + i)] = (uint8_t)name[i]; if (!name[i]) break; }
    }
    if (d) closedir(d);
    if (!found) return 0;                                            /* 79B2: findfirst failed, di = 0 */
    /* 0x79EA: for i = 0..di, for j = i..di: if strcmp(name[i], name[j]) > 0 swap (strcpy through a temporary) */
    for (int i = 0; i <= di; i++)
        for (int j = i; j <= di; j++) {
            uint16_t a = (uint16_t)(FILE_LIST + 13 * i), b = (uint16_t)(FILE_LIST + 13 * j);
            if (ds_strcmp(a, b) > 0) {
                char tmp[0x16];
                int n = 0;
                while (n < (int)sizeof(tmp) - 1 && g_ds[(uint16_t)(a + n)]) { tmp[n] = (char)g_ds[(uint16_t)(a + n)]; n++; }
                tmp[n] = 0;
                ds_strcpy(a, b);
                for (int t = 0; t <= n; t++) g_ds[(uint16_t)(b + t)] = (uint8_t)tmp[t];
            }
        }
    return di;
}

/* 0x7A54 read_descriptions(last): each listed file "s.csv" whose first 4 bytes are "10A" (DS:5B0E) gives slot
   s = name[0] - '0' (0..8) its description: 48 bytes (0x30) read from offset 4 into DS:A6C0 + 49*s ("" if that
   read fails). Verifier correction 2: the clear loop 0x7A5E indexes with an uninitialised local, so the original
   clears at most one random slot; the port clears all 9 (GAMELOOP_NOTES, corrections). */
static void descriptions7a54(int last) {
    for (int s = 0; s < 9; s++) ds_strcpy((uint16_t)(SLOT_DESC + 49 * s), 0x5CEA);
    for (int k = 0; k <= last; k++) {
        uint16_t di = (uint16_t)(FILE_LIST + 13 * k);
        FILE *f = fopen(host_name(dsp(di)), "rb");                    /* 0x9EA6 _dos_open(name, 0) */
        if (!f) continue;
        if (dos_read(f, HDR_BUF, 4) == 0 && ds_strcmp(0x5B0E, HDR_BUF) == 0) {
            int si = (int)(int8_t)g_ds[di] - 0x30;
            if (si >= 0 && si < 9) {
                uint16_t slot = (uint16_t)(SLOT_DESC + 49 * si);
                if (dos_read(f, slot, 0x30) != 0) ds_strcpy(slot, 0x5CEB);
            }
        }
        fclose(f);                                                   /* 0x9F90 */
    }
}

/* the slot screen shared by 0x7784 (save) and 0x78F0 (load), up to 0x54C8: the whole screen saved (691B), the
   record DS:5BCC drawn into the work buffer (67E4) with title W(5BD8) = title, the 9 descriptions in colour 0 at
   (65, 63 + 10*s) clipped to DS:5C42 = (36,46)-(284,153) (copied into the work buffer's clip rect DS:C0DA), back to
   the screen (680F) and the whole work buffer copied to it (6973); then 54C8(5BCC, 0). Returns 0..8 or 0xFF. */
static int slot_screen(uint16_t title, menu_t *m) {
    descriptions7a54(list7996());
    ui_copy_rect(g_wrk, g_scr, FULL);                                 /* 691B(0, 0, 319, 199) */
    if (W(0x9B26) != 6) for (int i = 0; i < 18; i++) B(0xC0D0 + i) = B(0xC1DA + i);   /* 67E4: draw into the work buffer */
    setW(0x5BD8, title);
    menu_load_ds(m, 0x5BCC);
    ui_menu_draw_mode(g_wrk, m, 2, -1);                               /* 5614(5BCC, 2) into the work buffer */
    for (int i = 0; i < 8; i++) B(0xC0DA + i) = B(0x5C42 + i);       /* clip rect */
    setW(0x9B69, 0);
    {
        static uint8_t tmp[64000];
        memcpy(tmp, g_wrk, sizeof(tmp));
        for (uint16_t y = 0x3F, p = SLOT_DESC; p < 0xA879; y += 10, p += 49)
            ui_text_attr(tmp, 0x41, y, dsp(p), W(0x9B69));            /* 899C(desc, 65, y) */
        ui_box_t clip = { SW(0xC0DA), SW(0xC0DC), SW(0xC0DE), SW(0xC0E0) };
        ui_copy_rect(g_wrk, tmp, clip);                               /* 8E62 clips each glyph to the rect */
    }
    for (int i = 0; i < 18; i++) B(0xC0D0 + i) = B(0x9AB6 + i);       /* 680F: the screen's descriptor again */
    ui_copy_rect(g_scr, g_wrk, FULL);                                 /* 6973(0, 0, 319, 199) */
    int r = menu_choose(m, -1, -1);                                   /* 54C8(5BCC, 0) */
    return r < 0 ? 0xFF : r;
}

/* 0x358A: wait for a key (0x03AA until non-zero). The port's input may end (script / window closed): Esc then. */
static int key358a(void) {
    for (;;) {
        int c = key_poll();
        if (c) return c;
        if (screen_quit()) return 0x1B;
        if (screen_scripted()) {                  /* a "click" line next: 358A does not read the mouse, so it is dropped */
            if (wait_input() == 0 && B(0xC07F)) return B(0xC07F);
            continue;
        }
        platform_sleep_ms(5);
    }
}

/* 0x36ED edit_line(buf, &xy, max, xmax) (Pascal): the text at xy in colour [9B69], a block cursor of the width of 'M'
   ([[9B56] + 0x2D]) and 1 + [9B5C] + 1 rows in colour 15 after it; printable keys and other control codes are appended
   while len < max and the cursor stays left of xmax - wM + x - 1; Backspace removes the last one (a 0 written there);
   Enter returns -1, Esc 0. The fill colour [9AFA] on entry erases the cursor. */
static int edit36ed(uint16_t buf, int x, int y, int max, int xmax) {
    int wM = 0, yo, fh = 8;                       /* [9B5C] = 8 at run time (the port's font: 8 rows) */
    if (!font_glyph('M', &wM, &yo)) wM = 0;
    int ybot = fh + y + 1;                        /* [bp-0C] */
    uint16_t fill = W(0x9AFA);                    /* [bp-0E] */
    ui_text_attr(g_scr, x, y, dsp(buf), W(0x9B69));
    int cx = ui_text_advance(dsp(buf)) + x;       /* [bp-2] */
    setW(0x9AFA, 0xF);
    ui_rect(g_scr, cx, y, wM + 1, ybot - y + 1, (uint8_t)W(0x9AFA));
    int len = ds_strlen(buf);                     /* [bp-6] */
    for (;;) {
        uint8_t c = (uint8_t)key358a();
        if (c == 0x1B) return 0;
        if (c == 0x0D) return -1;
        if (c == 0x08) {                          /* 0x37D9 */
            int old = ui_text_advance(dsp(buf)) + x;
            if (--len < 0) len = 0;
            g_ds[(uint16_t)(buf + len)] = 0;
            cx = ui_text_advance(dsp(buf)) + x;
            setW(0x9AFA, fill);
            ui_rect(g_scr, cx, y, wM + old - cx + 1, ybot - y + 1, (uint8_t)W(0x9AFA));
        } else {                                  /* 0x376A */
            setW(0x9AFA, fill);
            ui_rect(g_scr, cx, y, wM + 1, ybot - y + 1, (uint8_t)W(0x9AFA));
            if (len < max && (uint16_t)(xmax - wM + x - 1) > (uint16_t)cx) {
                g_ds[(uint16_t)(buf + len)] = c;
                len++;
            }
            ui_text_attr(g_scr, x, y, dsp(buf), W(0x9B69));
            cx = ui_text_advance(dsp(buf)) + x;
        }
        setW(0x9AFA, 0xF);                        /* 0x37BD */
        ui_rect(g_scr, cx, y, wM + 1, ybot - y + 1, (uint8_t)W(0x9AFA));
    }
}

/* 0x7852 enter_description(s): DS:A8F0 cleared (0x30 bytes) and set to slot s's description; "Enter a Game Description"
   (DS:5B30) in white at (160, 46); [9AFA] = [6346]; 36ED(A8F0, {65, 63 + 10*s}, 0x30, 0xD5) in colour 0. Esc -> 0xFF;
   an empty text becomes "Untitled" (DS:5CDB). Returns s. */
static int describe7852(int s) {
    memset(&g_ds[DESC_BUF], 0, 0x30);
    ds_strcpy(DESC_BUF, (uint16_t)(SLOT_DESC + 0x31 * s));
    setW(0x9B69, 0xF);
    text8be4(g_scr, 0xA0, 0x2E, dsp(0x5B30), W(0x9B69));
    setW(0x9AFA, W(0x6346));
    setW(0x9B69, 0);
    if (!edit36ed(DESC_BUF, 0x41, 0x3F + 10 * s, 0x30, 0xD5)) return 0xFF;
    if (ds_strcmp(DESC_BUF, 0x5CDA) == 0) ds_strcpy(DESC_BUF, 0x5CDB);
    return s;
}

/* 0x7784 select_save_slot: the slot screen titled "Select a Save Position" (DS:5B18); after the choice the title is
   drawn again in [9B69] = [6348] (8BE4 at (160, 46)), then 7852 for a slot. Returns the slot or 0xFF. */
static int save_slot7784(void) {
    menu_t m;
    int r = slot_screen(0x5B18, &m);
    setW(0x9B69, W(0x6348));
    text8be4(g_scr, 0xA0, 0x2E, dsp(0x5B18), W(0x9B69));
    if (r != 0xFF) r = describe7852(r);
    return r;
}

/* 0x78F0 select_load_slot: the slot screen titled "Select a Game to Load" (DS:5B4A). Returns 54C8's value. */
static int load_slot78f0(void) {
    menu_t m;
    return slot_screen(0x5B4A, &m);
}

/* 0x762D save_game */
static void save762d(void) {
    disk_prompt7b1e(W(0x5C4A));
    int si = save_slot7784();
    if (si != 0xFF) {
        B(FILE_NAME) = (uint8_t)(si + 0x30);
        char up[13];                                                  /* DOS stores the name in upper case: "N.CSV" */
        size_t k;
        for (k = 0; k < 12 && g_ds[(uint16_t)(FILE_NAME + k)]; k++) {
            char c = (char)g_ds[(uint16_t)(FILE_NAME + k)];
            up[k] = (c >= 'a' && c <= 'z') ? (char)(c - 0x20) : c;
        }
        up[k] = 0;
        FILE *f = fopen(up, "wb");                                    /* 0xA1B9 _dos_creat(name, 0) */
        if (!f) message_box(dsp(0x5C72));                             /* "Save Failed\nCould not create file" */
        else {
            int n = pack75a3();
            int err = dos_write(f, PACK_BUF, (unsigned)n);            /* 0x7688: si = 9EC5(...) */
            fclose(f);                                                /* 0x7693 9F90: the result is not used */
            if (err) message_box(dsp(0x5C51));                        /* "Save Failed\nCould not write file" */
        }
    }
    disk_prompt7b1e(W(0x5C4C));
}

/* 0x7737 load_file(name): open; n = 75A3() (the pack also fills AC60 with the current state); read n bytes into AC60
   (the result is not checked); close; 75E8 unpack. Open error: "Load Failed\nCould not open file" (DS:5CB4). */
static void load7737(uint16_t name) {
    FILE *f = fopen(dsp(name), "rb");
    if (!f) { message_box(dsp(0x5CB4)); return; }
    int n = pack75a3();
    (void)dos_read(f, PACK_BUF, (unsigned)n);
    fclose(f);
    unpack75e8();
}

/* 0x76C2 load_game */
static void load76c2(void) {
    disk_prompt7b1e(W(0x5C4A));
    int si = load_slot78f0();
    if (si != 0xFF) {
        if (ds_strcmp((uint16_t)(SLOT_DESC + 0x31 * si), 0x5C94) == 0)
            message_box(dsp(0x5C95));                                 /* "No saved game\nat that position" */
        else {
            B(FILE_NAME) = (uint8_t)(si + 0x30);
            load7737(FILE_NAME);
        }
    }
    disk_prompt7b1e(W(0x5C4C));
}

/* ---- difficulty ---- */
/* 0x1A5E: the four module levels, name DS:W(0484 + 2*B(048E+i)) at (195, 102 + 10*i) in colour 15 */
static void show_levels1a5e(void) {
    setW(0x9B69, 0xF);
    for (int si = 0, di = 0x66; si < 4; si++, di += 10)
        ui_text_attr(g_scr, 0xC3, di, dsp(W(0x484 + 2 * B(0x48E + si))), W(0x9B69));
}

/* 0x19BC fine_tuning: menu DS:05CA (Land Battle, Racing Chariots, Gladiatorial Combat, Naval Battle, Continue) with
   the levels; a module opens DS:05DC titled item text + " Difficulty Menu" (DS:0426, built in DS:A64C) with its level
   highlighted; a choice 0..3 sets DS:048E[module]. */
static void fine_tuning19bc(void) {
    int di = 0;
    menu_t m, sub;
    for (;;) {
        menu_load_ds(&m, 0x5CA);
        ui_menu_draw_mode(g_scr, &m, 2, -1);                         /* 5614(05CA, 2) */
        show_levels1a5e();
        int r = menu_choose(&m, -1, -1);                              /* 54C8(05CA, di) */
        di = r < 0 ? 0xFF : r;
        if (di == 4 || di == 0xFF) return;
        int mod = di;                                                 /* [bp-2] */
        ds_strcpy(0xA64C, W((uint16_t)(W(0x5D4) + 12 * di)));
        ds_strcat(0xA64C, 0x426);
        menu_load_ds(&sub, 0x5DC);
        int si = B(0x48E + di);
        r = menu_choose(&sub, 2, si);                                 /* 5614(05DC, 2); 585B(05DC, si, ON); 54C8 */
        si = r < 0 ? 0xFF : r;
        if (si < 4) B(0x48E + mod) = (uint8_t)si;
    }
}

/* 0x1B32 (inline in 0x1A8B) difficulty: menu DS:05B8 (Galley Slave, Legionnaire, Senator, Emperor, Fine Tuning, Continue)
   with the common level highlighted, or Fine Tuning when the four differ; 0..3 sets all four, 4 runs 0x19BC and comes
   back, 5 / Esc leave. */
static void difficulty1b32(void) {
    menu_t m;
    menu_load_ds(&m, 0x5B8);
    ui_menu_draw_mode(g_scr, &m, 2, -1);                             /* 5614(05B8, 2) */
    for (;;) {
        int di = B(0x48E);
        for (int si = 1; si < 4; si++) if (B(0x48E + si) != B(0x48E)) di = 4;
        int r = menu_choose(&m, -1, di);                              /* 585B(05B8, di, ON); 54C8(05B8, di) */
        di = r < 0 ? 0xFF : r;
        if (di == 4) {                                                /* 0x1B9C */
            fine_tuning19bc();
            menu_load_ds(&m, 0x5B8);
            ui_menu_draw_mode(g_scr, &m, 2, -1);
            continue;
        }
        if (di != 0xFF && di != 5)
            for (int bx = 0; bx < 4; bx++) B(0x48E + bx) = (uint8_t)di;
        return;
    }
}

/* ---- 0x1A8B game_control_menu (year button / Esc on the map) ---- */
int game_control_menu(void) {
    int quit = 0;                                                     /* [bp-6] */
    ui_copy_rect(g_wrk, g_scr, FULL);                                 /* 691B(0, 0, 319, 199) */
    menu_t m;
    menu_load_ds(&m, 0x5A6);
    int r = menu_choose(&m, 2, -1);                                   /* 5614(05A6, 2); 54C8(05A6, 0) */
    unsigned c = r < 0 ? 0xFF : (unsigned)r;
    if (c <= 5) switch (c) {                                          /* jump table 0x1ADD */
    case 0: save762d(); break;                                        /* 0x1AE9 */
    case 1: load76c2(); break;                                        /* 0x1AEF */
    case 2: new_game(); break;                                        /* 0x1AF5 */
    case 3: {                                                         /* 0x1AFB About */
        ui_box_t box = { SW(0x5A6), SW(0x5A8), SW(0x5AA), SW(0x5AC) };
        ui_copy_rect(g_scr, g_wrk, box);                              /* 6973: the menu goes */
        ds_strcpy(PACK_BUF, 0x5EE);                                   /* "CENTURION\nDefender of Rome\n..." */
        ds_strcat(PACK_BUF, 0x46E);                                   /* "Version 1.00A 5/25/90" */
        message_box(dsp(PACK_BUF));
        break;
    }
    case 4: difficulty1b32(); break;                                  /* 0x1B32 */
    case 5: if (yes_no(dsp(0x672))) quit = -1; break;                 /* 0x1BC9 "Really Quit?" */
    }
    return quit;                                                      /* the caller redraws the map (0x494A(0xFF)) */
}
