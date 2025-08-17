// This file contains a simple implementation of a linked list in C.

#include <stdlib.h>
#include <stdio.h>
#include "list.h"


list *list_create(void) {
	list *newlist = malloc(sizeof(list));
	newlist->head = newlist->tail = NULL;
	newlist->size = 0;

	return newlist;
}

void list_destroy(list *lst) {
	node *node, *next;

	node = lst->head;

	while (node->next != NULL) {
		next = node->next;
		free(node);
		node = next;
	}

	free(lst);
}

/* Operations */
int  list_find(const list *lst, size_t val) {

	for (node *node = lst->head; node->next != NULL; node = node->next) {
		if (node->val == val) {
			return 1;
		}
	}

	return 0;
}

// This function allocates a node, if list is NULL it creates a new one. Returns
// the updated list. Add always inserts in the tail. Does not check for
// duplicates. Insertion is O(1):
void list_push_back(list *list, size_t val)
{
	struct list *newlist;

	if (!list)
	{
		newlist = malloc(sizeof(struct list));
		newlist->size = 0;
	}
	else
	{
		newlist = list;
	}

	struct node *new_node = (struct node *)malloc(sizeof(struct node));

	new_node->val = val;
	new_node->next = NULL;

	if (newlist->size == 0)
	{
		newlist->head = newlist->tail = new_node;
	}
	else
	{
		newlist->tail = newlist->tail->next = new_node;
	}

	newlist->size++;
}


void node_match(struct node* node, void *params, void **result) {
	struct node **result_casted = (struct node **) result;
	
	if (node->val == ((size_t ) params)) {
		*result_casted = node;
	}
}

void node_print(struct node* node, void* params, void **result) {
	printf("Node: %zu\n", node->val);
}

void for_each_list(const list *list, void (*fn)(struct node *, void *params, void **result), void *params, void **result) {

	// Check if result is not null before de-referencing
	if (result) {
		*result = NULL;
	}

	for (struct node * node = list->head; node != NULL; node = node->next) {
		fn(node, params, result);
	}
}

void print_list(const list *list) {
	printf("Size of the list is %zu\n", list->size);
	for_each_list(list, node_print, NULL, NULL);
}