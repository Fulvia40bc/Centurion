/* The shared vector runner of vecrun.h (TASK 15/16). See vecrun.h for the file format and the checks. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/state.h"
#include "vecrun.h"

static uint8_t *g_copy;
static long g_run, g_failed;               /* file vectors */
static long g_hand_run, g_hand_failed;     /* hand cases */
static long g_declared = -1;
static long *g_per_run, *g_per_fail;
static int g_detail;                       /* FAIL details left */

int vec_init(void)
{
    int n = vec_func_count();

    g_copy = malloc(DS_SIZE);
    g_per_run = calloc((size_t)n, sizeof *g_per_run);
    g_per_fail = calloc((size_t)n, sizeof *g_per_fail);
    return g_copy != NULL && g_per_run != NULL && g_per_fail != NULL ? 0 : -1;
}

static void run_vector(const vec_t *v, int hand)
{
    int id = vec_func_id(v->func);
    int i, ret = 0, bad = 0;
    uint32_t d;

    if (id < 0)
        return;
    memset(g_ds, 0, DS_SIZE);
    for (i = 0; i < v->nm; i++)
        memcpy(DP(v->m[i].addr), v->m[i].b, (size_t)v->m[i].len);
    memcpy(g_copy, g_ds, DS_SIZE);

    (void)vec_call(id, v->a, v->b, &ret);

    if (v->has_ret && ret != v->ret) {
        bad = 1;
        if (g_detail < 20) {
            printf("FAIL %s: returned %d, expected %d\n", v->tline, ret, v->ret);
            g_detail++;
        }
    }
    for (i = 0; i < v->nc; i++)
        memcpy(g_copy + (v->c[i].addr - DS_BASE), v->c[i].b, (size_t)v->c[i].len);
    /* g_copy now holds the expected data object (M lines applied, then the C lines) */
    for (d = 0; d < DS_SIZE; d++)
        if (g_ds[d] != g_copy[d]) {
            bad = 1;
            if (g_detail < 20) {
                printf("FAIL %s: byte 0x%05X is %02X, expected %02X\n", v->tline, (unsigned)(d + DS_BASE),
                       g_ds[d], g_copy[d]);
                g_detail++;
            }
            break;
        }

    if (hand) {
        g_hand_run++;
        if (bad)
            g_hand_failed++;
    } else {
        g_run++;
        if (bad)
            g_failed++;
    }
    g_per_run[id]++;
    if (bad)
        g_per_fail[id]++;
}

void vec_run_vector(const vec_t *v) { run_vector(v, 0); }
void vec_hand_vector(const vec_t *v) { run_vector(v, 1); }

static int hexval(int c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* parse a hex string of 2..64 digits into out; returns the byte count or -1 */
static int parse_hex(const char *s, uint8_t *out)
{
    int n = 0, hi, lo;

    while (s[0] != 0 && s[1] != 0) {
        hi = hexval((unsigned char)s[0]);
        lo = hexval((unsigned char)s[1]);
        if (hi < 0 || lo < 0 || n >= VEC_MAXBYTES)
            return -1;
        out[n++] = (uint8_t)(hi * 16 + lo);
        s += 2;
    }
    if (s[0] != 0 || n < 1)
        return -1;
    return n;
}

long vec_run_file(const char *path)
{
    FILE *f = fopen(path, "r");
    static vec_t cur;                 /* too big for the stack with VEC_MAXMEM 512 */
    char line[256], *tok;
    int in_vec = 0, lineno = 0;

    g_declared = -1;
    if (f == NULL) {
        fprintf(stderr, "vecrun: cannot open %s\n", path);
        return -1;
    }
    while (fgets(line, sizeof line, f) != NULL) {
        size_t len = strlen(line);
        lineno++;
        if (len == sizeof line - 1 && line[len - 1] != '\n') {
            fprintf(stderr, "vecrun: %s line %d longer than 200 characters\n", path, lineno);
            fclose(f);
            return -1;
        }
        if (len > 0 && line[len - 1] == '\n')
            line[--len] = 0;
        if (len > 0 && line[len - 1] == '\r')
            line[--len] = 0;
        {
            char *h = strchr(line, '#');
            if (h != NULL)
                *h = 0;
        }
        tok = strtok(line, " \t");
        if (tok == NULL)
            continue;
        if (strcmp(tok, "N") == 0) {
            if (g_declared >= 0 || in_vec || g_run > 0) {
                fprintf(stderr, "vecrun: %s line %d: N must come once, before the first vector\n", path, lineno);
                fclose(f);
                return -1;
            }
            tok = strtok(NULL, " \t");
            if (tok == NULL) { fclose(f); return -1; }
            g_declared = strtol(tok, NULL, 10);
        } else if (strcmp(tok, "T") == 0) {
            int id;
            if (in_vec) {
                fprintf(stderr, "vecrun: %s line %d: T inside a vector (missing E)\n", path, lineno);
                fclose(f);
                return -1;
            }
            tok = strtok(NULL, " \t");
            if (tok == NULL) { fclose(f); return -1; }
            id = vec_func_id(tok);
            if (id < 0) {
                fprintf(stderr, "vecrun: %s line %d: unknown function %s\n", path, lineno, tok);
                fclose(f);
                return -1;
            }
            memset(&cur, 0, sizeof cur);
            snprintf(cur.func, sizeof cur.func, "%s", vec_func_name(id));
            cur.has_ret = 1;
            snprintf(cur.tline, sizeof cur.tline, "T %s", vec_func_name(id));
            {
                int k, nargs = vec_func_nargs(id);
                char *args[2] = { NULL, NULL };
                for (k = 0; k < nargs; k++) {
                    args[k] = strtok(NULL, " \t");
                    if (args[k] == NULL) {
                        fprintf(stderr, "vecrun: %s line %d: %s wants %d args\n", path, lineno,
                                vec_func_name(id), nargs);
                        fclose(f);
                        return -1;
                    }
                    strcat(cur.tline, " ");
                    strcat(cur.tline, args[k]);
                }
                cur.a = args[0] != NULL ? (int)strtol(args[0], NULL, 10) : 0;
                cur.b = args[1] != NULL ? (int)strtol(args[1], NULL, 10) : 0;
            }
            in_vec = 1;
        } else if (strcmp(tok, "M") == 0 || strcmp(tok, "C") == 0) {
            vec_mem *dst;
            int is_m = tok[0] == 'M';
            char *at, *hx;
            int n;
            if (!in_vec) {
                fprintf(stderr, "vecrun: %s line %d: %s outside a vector\n", path, lineno, tok);
                fclose(f);
                return -1;
            }
            at = strtok(NULL, " \t");
            hx = strtok(NULL, " \t");
            if (at == NULL || hx == NULL || strtok(NULL, " \t") != NULL) {
                fprintf(stderr, "vecrun: %s line %d: bad %s line\n", path, lineno, tok);
                fclose(f);
                return -1;
            }
            if ((is_m ? cur.nm : cur.nc) >= VEC_MAXMEM) {
                fprintf(stderr, "vecrun: %s line %d: too many %s lines\n", path, lineno, tok);
                fclose(f);
                return -1;
            }
            dst = is_m ? &cur.m[cur.nm] : &cur.c[cur.nc];
            dst->addr = (uint32_t)strtoul(at, NULL, 16);
            n = parse_hex(hx, dst->b);
            if (n < 0) {
                fprintf(stderr, "vecrun: %s line %d: bad hex (1..32 bytes)\n", path, lineno);
                fclose(f);
                return -1;
            }
            dst->len = n;
            if (is_m)
                cur.nm++;
            else
                cur.nc++;
        } else if (strcmp(tok, "R") == 0) {
            char *rt;
            if (!in_vec) { fclose(f); return -1; }
            rt = strtok(NULL, " \t");
            if (rt == NULL) { fclose(f); return -1; }
            if (strcmp(rt, "void") == 0)
                cur.has_ret = 0;
            else
                cur.ret = (int)strtol(rt, NULL, 10);
        } else if (strcmp(tok, "E") == 0) {
            if (!in_vec) {
                fprintf(stderr, "vecrun: %s line %d: E outside a vector\n", path, lineno);
                fclose(f);
                return -1;
            }
            run_vector(&cur, 0);
            in_vec = 0;
        } else {
            fprintf(stderr, "vecrun: %s line %d: unknown directive %s\n", path, lineno, tok);
            fclose(f);
            return -1;
        }
    }
    fclose(f);
    if (in_vec) {
        fprintf(stderr, "vecrun: %s: last vector not ended with E\n", path);
        return -1;
    }
    if (g_declared >= 0 && g_run != g_declared)
        printf("vecrun: ran %ld vectors, N declared %ld\n", g_run, g_declared);
    return g_declared >= 0 ? g_declared : g_run;
}

void vec_print_summary(const char *prog)
{
    int i, n = vec_func_count();

    for (i = 0; i < n; i++)
        printf("%s: %ld run, %ld failed\n", vec_func_name(i), g_per_run[i], g_per_fail[i]);
    printf("hand cases: %ld run, %ld failed\n", g_hand_run, g_hand_failed);
    printf("%s: %ld vectors, %ld failed\n", prog, g_run, g_failed);
}

int vec_exit_status(void)
{
    return g_failed == 0 && g_hand_failed == 0 && g_run == g_declared ? 0 : 1;
}
