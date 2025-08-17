#include <stdio.h>
#include <stdlib.h>
#include "skiplist.h"

// Initializes the skiplist with a maximum level and a given probability of
// promoting a node to upper level
struct skiplist *init(size_t max_levels, float p, unsigned int seed) {
    srand(seed);
    struct skiplist *newskiplist = malloc(sizeof(struct skiplist));

    newskiplist->max_levels = max_levels;
    newskiplist->curr_lvl = 0;

    // Create the root node
    struct skiplist_node *root = malloc(sizeof(struct skiplist_node));
    root->val; // Don't care
    root->nexts = malloc(sizeof(*(root->nexts)) * newskiplist->max_levels);
    root->nexts_len = newskiplist->max_levels;

    for (size_t i = 0; i < newskiplist->max_levels; i++) {
        root->nexts[i] = NULL;
    }

    newskiplist->root = root;
    newskiplist->p = p;

    return newskiplist;
}

static size_t promotions(struct skiplist *list) {
    size_t prom = 0;
    while ((rand() < list->p*RAND_MAX) && prom < list->max_levels) {
        prom++;
    }

    prom = (prom == 0) ? 1 : prom;
    prom = (prom > list->max_levels) ? list->max_levels : prom;
    
    return prom;
}

// Adds a new sorted node into the skiplist
struct skiplist *add(struct skiplist *skiplist, size_t val) {
    struct skiplist_node *node = skiplist->root;
    
    // printf("[DEBUG] Inserting %zu\n", val);

    struct skiplist_node **past_nodes = malloc(sizeof(struct skiplist_node *) * skiplist->max_levels);
    
    // Initialize all of them
    for (size_t lvl = 0; lvl < skiplist->max_levels; lvl++) {
        past_nodes[lvl] = skiplist->root;
    }

    // Start from current-level and search where to insert the node
    for (int lvl = (int) skiplist->curr_lvl; lvl >= 0; lvl--) {
        // printf("[DEBUG] Level %d\n", lvl);
        // Iterate the current list until the next node is not NULL or the next
        // val is <= val
        while (node->nexts[lvl] != NULL && node->nexts[lvl]->val <= val) {

            // printf("[DEBUG] Found %zu\n", node->nexts[lvl]->val);
            if (lvl == 0 && node->nexts[lvl]->val == val) {
                // printf("[DEBUG] Duplicate found\n", lvl);
                // Early exit, do not add duplicates
                return skiplist;
            }

            // Move to next node
            node = node->nexts[lvl];
        }
        past_nodes[lvl] = node;
    }

    // Now create the new node
    struct skiplist_node *new_node = malloc(sizeof(struct skiplist_node));
    new_node->val = val;
    new_node->nexts_len = 0;
    new_node->nexts = NULL;
    size_t prom = promotions(skiplist);
    new_node->nexts_len = prom;
    new_node->nexts = malloc(sizeof(struct skiplist_node*) * new_node->nexts_len);

    for (size_t lvl = 0; lvl < prom; lvl++) {
        new_node->nexts[lvl] = past_nodes[lvl]->nexts[lvl];
        past_nodes[lvl]->nexts[lvl] = new_node;
        skiplist->curr_lvl = (lvl > skiplist->curr_lvl) ? lvl : skiplist->curr_lvl;
    }


    free(past_nodes);
    
    return skiplist;
}

void debug_traverse(struct skiplist *skiplist, size_t lvl) {
    printf("Beginning of traversal of list at level: %zu\n", lvl);
    for (   struct skiplist_node *node = skiplist->root->nexts[lvl]; 
            node != NULL; 
            node = node->nexts[lvl]
        ) 
    {
        printf("%zu, ", node->val);
    }
    printf("\n");
}

int search(struct skiplist* skiplist, size_t val) {
    struct skiplist_node *node = skiplist->root;
    
    // Start from current-level and search where to insert the node
    for (int lvl = (int) skiplist->curr_lvl; lvl >= 0; lvl--) {
        // printf("[DEBUG] Level %d\n", lvl);
        // Iterate the current list until the next node is not NULL or the next
        // val is <= val
        while (node->nexts[lvl] != NULL && node->nexts[lvl]->val <= val) {
            
            if (node->nexts[lvl]->val == val) {
                // Early exit, do not add duplicates
                return 1;
            }
            
            // Move to next node
            node = node->nexts[lvl];
        }
    }
    
    return 0;
}

void destroy(struct skiplist *skiplist) {
    struct skiplist_node *node, *next;
    node = skiplist->root->nexts[0];

    if (node == NULL) {
        return;
    }

    while (node->nexts[0] != NULL) {
        next = node->nexts[0];
        free(node->nexts);
        free(node);
        node = next;
    }

    free(skiplist->root);
    free(skiplist);
}

struct skiplist *delete(struct skiplist *skiplist, size_t val) {
    struct skiplist_node *node = skiplist->root;
    struct skiplist_node *to_be_freed = NULL;
    
    // Start from current-level and search where to insert the node
    for (int lvl = (int) skiplist->curr_lvl; lvl >= 0; lvl--) {
        while (node->nexts[lvl] != NULL && node->nexts[lvl]->val <= val) {
            if (node->nexts[lvl]->val == val) {
                if (lvl == 0) to_be_freed = node->nexts[lvl];

                node->nexts[lvl] = node->nexts[lvl]->nexts[lvl];

                break;
            }
            
            // Move to next node
            node = node->nexts[lvl];
        }
    }

    if (to_be_freed != NULL) {
        free(to_be_freed->nexts);
        free(to_be_freed);
    }

    // Check the current height of the list
    for (int i = skiplist->curr_lvl; i > 0; i--) {
        if (skiplist->root->nexts[i] == NULL) {
            skiplist->curr_lvl--;
        }
    }
    


    return skiplist;
}

void print_skiplist(struct skiplist *skiplist) {
    printf("Info for skiplist:\n");
    printf("Probability of promotion: %.1f\n", skiplist->p);
    printf("Level: %zu/%zu\n", skiplist->curr_lvl+1, skiplist->max_levels);

    for (size_t i = 0; i < skiplist->curr_lvl+1; i++)
    {
        debug_traverse(skiplist, i);
    }
    
}


#define _search(skiplist, val) \
printf("Searching for %zu: %s\n", val, search(skiplist, val) ? "Found" : "Not Found");


// int main(void) {
    
//     struct skiplist *skiplist = init(5, 0.5, 0);
//     for (size_t i = 0; i < 10; i++)
//     {
//         skiplist = add(skiplist, i);
//     }

//     for (size_t i = 0; i < 10; i++)
//     {
//         skiplist = delete(skiplist, i);
//     }

//     skiplist = delete(skiplist, 1);
//     skiplist = delete(skiplist, 0);
//     _search(skiplist, 1);

//     print_skiplist(skiplist);

//     for (size_t i = 0; i < 10; i++)
//     {
//         skiplist = add(skiplist, i);
//     }

//     print_skiplist(skiplist);

//     destroy(skiplist);

// 	return 0;
// }