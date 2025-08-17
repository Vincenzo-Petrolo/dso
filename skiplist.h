#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <stddef.h>

/* Public types mirror your implementation so existing .c compiles unchanged. */
typedef struct skiplist_node {
    struct skiplist_node **nexts;
    size_t nexts_len;
    size_t val;
} skiplist_node;

typedef struct skiplist {
    size_t max_levels;
    size_t curr_lvl;
    struct skiplist_node *root;
    float p;
} skiplist;

/* Constructors / destructors */
skiplist *init(size_t max_levels, float p, unsigned int seed);
void destroy(skiplist *sl);

/* Operations */
skiplist *add(skiplist *sl, size_t val);      /* returns sl for chaining */
int search(skiplist *sl, size_t val);         /* 1 if found, 0 otherwise */
skiplist *delete(skiplist *sl, size_t val);   /* optional: remove one key */

/* Debug / diagnostics (optional) */
void print_skiplist(skiplist *sl);

#endif /* SKIPLIST_H */
