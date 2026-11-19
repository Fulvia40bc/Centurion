#ifndef VECRUN_H
#define VECRUN_H

/* Shared vector runner for the port's table-driven tests (TASK 15 formulas_test, TASK 16 helpers2_test).
   A vector file is ASCII text, LF or CRLF, lines at most 200 characters, '#' comments to end of line, blank
   lines skipped:
     N <count>            once, before the first vector
     T <func> [<arg>...]  starts a vector; args are signed decimal int32 in register order (eax, edx)
     M <addr> <hex>       addr is a hex link address (no 0x); hex is 1..32 bytes, two digits each; stored at
                          addr before the call
     R <int> | R void     the expected return (void for functions that return nothing)
     C <addr> <hex>       after the call these bytes must hold these values
     E                    end of the vector: run it
   Per vector: zero all DS_SIZE bytes, apply the M lines, take a copy, call, compare the return, apply the C
   lines to the copy, memcmp the whole data object with the copy (every byte outside the C lines must be
   unchanged). FAIL lines are printed (at most 20 in detail), then a count per function and a final
   "<prog>: <run> vectors, <failed> failed" line; exit status 0 only when nothing failed and run == N.

   The test using this provides the function table (vec_func_*) and the call (vec_call), sets
   g_ds = calloc(DS_SIZE + 0x1000, 1) itself (never calls state_init) and calls vec_init() first. */

#include <stdint.h>

#define VEC_MAXMEM 512
#define VEC_MAXBYTES 32

typedef struct {
    uint32_t addr;
    int len;
    uint8_t b[VEC_MAXBYTES];
} vec_mem;

typedef struct {
    char func[16];
    int a, b;
    vec_mem m[VEC_MAXMEM];
    int nm;
    vec_mem c[VEC_MAXMEM];
    int nc;
    int has_ret, ret;        /* has_ret 0 = void */
    char tline[200];         /* the T line, for FAIL messages */
} vec_t;

/* provided by the test */
int  vec_func_count(void);
const char *vec_func_name(int id);
int  vec_func_nargs(int id);                       /* argument count of the function's prototype */
int  vec_func_id(const char *name);                /* -1 unknown */
int  vec_call(int id, int a, int b, int *ret);     /* returns 1 when the function returns void */

/* provided by vecrun.c */
int  vec_init(void);                               /* 0 ok; allocates the copy buffer and counters */
void vec_run_vector(const vec_t *v);               /* a file vector */
void vec_hand_vector(const vec_t *v);              /* a hand case (counted separately) */
long vec_run_file(const char *path);               /* N declared, or -1 on a parse error */
void vec_print_summary(const char *prog);          /* per-function counts, hand line, final line */
int  vec_exit_status(void);                        /* 0 when nothing failed and run == N */

#endif
