#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "skiplist.h"
#include "list.h"

/* -------- timing helpers -------- */
static double now_seconds(void) {
    /* Portable enough: CPU time used by the process (seconds). */
    return (double)clock() / (double)CLOCKS_PER_SEC;
}

static void shuffle(size_t *a, size_t n, unsigned int seed) {
    srand(seed);
    for (size_t i = n; i > 1; --i) {
        size_t j = (size_t)(rand() % i);
        size_t tmp = a[i-1]; a[i-1] = a[j]; a[j] = tmp;
    }
}

/* -------- workloads -------- */
typedef struct {
    size_t n_insert;
    size_t n_search_hit;
    size_t n_search_miss;
    unsigned int seed;
} params;

static void gen_keys(size_t *seq, size_t n) {
    for (size_t i = 0; i < n; ++i) seq[i] = i;
}

static double bench_skiplist(const params *p) {
    skiplist *sl = init(20, 0.5f, p->seed);
    size_t *ins = malloc(p->n_insert * sizeof(size_t));
    size_t *qhit = malloc(p->n_search_hit * sizeof(size_t));
    size_t *qmiss = malloc(p->n_search_miss * sizeof(size_t));

    gen_keys(ins, p->n_insert);
    shuffle(ins, p->n_insert, p->seed ^ 0xA5A5A5A5u);

    /* hits are keys in range [0, n_insert) shuffled */
    gen_keys(qhit, p->n_search_hit);
    shuffle(qhit, p->n_search_hit, p->seed ^ 0xC3C3C3C3u);

    /* misses are from range [n_insert, 2*n_insert) */
    for (size_t i = 0; i < p->n_search_miss; ++i) qmiss[i] = p->n_insert + i;

    double t0, t1, t_ins, t_hit, t_miss;

    t0 = now_seconds();
    for (size_t i = 0; i < p->n_insert; ++i) add(sl, ins[i]);
    t1 = now_seconds(); t_ins = t1 - t0;

    volatile int sink = 0;

    t0 = now_seconds();
    for (size_t i = 0; i < p->n_search_hit; ++i) sink ^= search(sl, qhit[i]);
    t1 = now_seconds(); t_hit = t1 - t0;

    t0 = now_seconds();
    for (size_t i = 0; i < p->n_search_miss; ++i) sink ^= search(sl, qmiss[i]);
    t1 = now_seconds(); t_miss = t1 - t0;

    printf("[skiplist] inserts: %.3f ms, search hit: %.3f ms, search miss: %.3f ms (ignore=%d)\n",
           t_ins*1000.0, t_hit*1000.0, t_miss*1000.0, sink);

    free(ins); free(qhit); free(qmiss);
    destroy(sl);
    return t_ins + t_hit + t_miss;
}

static double bench_list(const params *p) {
    list *lst = list_create();
    size_t *ins = malloc(p->n_insert * sizeof(size_t));
    size_t *qhit = malloc(p->n_search_hit * sizeof(size_t));
    size_t *qmiss = malloc(p->n_search_miss * sizeof(size_t));

    gen_keys(ins, p->n_insert);
    shuffle(ins, p->n_insert, p->seed ^ 0x5E5E5E5Eu);

    gen_keys(qhit, p->n_search_hit);
    shuffle(qhit, p->n_search_hit, p->seed ^ 0x9D9D9D9Du);

    for (size_t i = 0; i < p->n_search_miss; ++i) qmiss[i] = p->n_insert + i;

    double t0, t1, t_ins, t_hit, t_miss;

    t0 = now_seconds();
    for (size_t i = 0; i < p->n_insert; ++i) list_push_back(lst, ins[i]);
    t1 = now_seconds(); t_ins = t1 - t0;

    volatile int sink = 0;
    t0 = now_seconds();
    for (size_t i = 0; i < p->n_search_hit; ++i) sink ^= list_find(lst, qhit[i]);
    t1 = now_seconds(); t_hit = t1 - t0;

    t0 = now_seconds();
    for (size_t i = 0; i < p->n_search_miss; ++i) sink ^= list_find(lst, qmiss[i]);
    t1 = now_seconds(); t_miss = t1 - t0;

    printf("[list]     inserts: %.3f ms, search hit: %.3f ms, search miss: %.3f ms (ignore=%d)\n",
           t_ins*1000.0, t_hit*1000.0, t_miss*1000.0, sink);

    free(ins); free(qhit); free(qmiss);
    list_destroy(lst);
    return t_ins + t_hit + t_miss;
}

int main(int argc, char **argv) {
    params p;
    p.n_insert = (argc > 1) ? (size_t)strtoull(argv[1], NULL, 10) : 100000;
    p.n_search_hit  = (argc > 2) ? (size_t)strtoull(argv[2], NULL, 10) : p.n_insert;
    p.n_search_miss = (argc > 3) ? (size_t)strtoull(argv[3], NULL, 10) : p.n_insert;
    p.seed = (argc > 4) ? (unsigned)strtoul(argv[4], NULL, 10) : 42u;

    printf("Benchmark with N_insert=%zu, N_hit=%zu, N_miss=%zu, seed=%u\n",
           p.n_insert, p.n_search_hit, p.n_search_miss, p.seed);

    double ts = bench_skiplist(&p);
    double tl = bench_list(&p);

    printf("\nSummary (wall-clock CPU time):\n");
    printf("  skiplist total: %.3f ms\n", ts*1000.0);
    printf("  list     total: %.3f ms\n", tl*1000.0);
    printf("Speedup (list/skiplist): %.2fx (lower is slower)\n", tl / (ts + 1e-12));

    return 0;
}
