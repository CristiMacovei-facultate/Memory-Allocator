// Nicolae-Cristian MACOVEI, Anul I, Grupa 312CAb

#ifndef DLL_H_GUARD
#define DLL_H_GUARD

/* ----------------------------------------------
implementation of a circular doubly linked list
---------------------------------------------- */

#include <stdlib.h>

typedef struct dll_node {
	struct dll_node *next;
	struct dll_node *prev;

	void *data;
} dll_node_t;

typedef struct doubly_linked_list {
	dll_node_t *head;

	int num_nodes;
	size_t block_size;
} dll_t;

dll_t *dll_create_empty(size_t block_size);

dll_t *dll_create_from_node(size_t block_size, dll_node_t *new_node);

dll_node_t *dll_find_first_if(dll_t *list, size_t target_addr);

void dll_insert_first(dll_t *list, dll_node_t *new_node);

void dll_insert_last(dll_t *list, dll_node_t *new_node);

void dll_insert_before_first_if(dll_t *list, dll_node_t *new_node);

dll_node_t *dll_pop_first(dll_t *list);

void dll_erase_node(dll_t *list, dll_node_t *node);

#endif
