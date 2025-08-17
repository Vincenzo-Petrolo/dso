#ifndef LIST_H
#define LIST_H

#include <stddef.h>

/* Public types aligned with your list.c */
typedef struct node {
    struct node *next;
    size_t val;
} node;

typedef struct list {
    struct node *head;
    struct node *tail;
    size_t size;
} list;

/* Constructors / destructors */
list *list_create(void);
void list_destroy(list *lst);

/* Operations */
void list_push_back(list *lst, size_t val);   /* O(1) append to tail */
int  list_find(const list *lst, size_t val);  /* 1 if found, else 0 */

/* Debug */
void print_list(const list *lst);

/*
 * NOTE:
 * If your existing list.c uses different names, add tiny wrappers or rename.
 * The benchmark uses only the functions above to keep things simple.
 */
#endif /* LIST_H */
