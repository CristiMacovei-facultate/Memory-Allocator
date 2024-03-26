#include <stdio.h>

#include "../dll.h"
#include "../structs.h"

void print_details(sfl_t *list)
{
	printf("Total memory: %lu bytes\n", list->total_mem);
	printf("Total allocated memory: %lu bytes\n", list->total_allocd);
	printf("Total free memory: %lu bytes\n",
		   list->total_mem - list->total_allocd);

	int num_free_blocks = 0;
	for (int i = 0; i < list->dlls->num_elements; ++i) {
		dll_t *dll = al_get(list->dlls, i);
		num_free_blocks += dll->num_nodes;
	}
	printf("Free blocks: %d\n", num_free_blocks);

	printf("Number of allocated blocks: %d\n",
		   list->allocd_blocks->num_elements);
	printf("Number of malloc calls: %d\n", list->num_allocs);
	printf("Number of fragmentations: %d\n", list->num_fragmentations);
	printf("Number of free calls: %d\n", list->num_frees);
}

void handle_print(sfl_t *list)
{
	printf("+++++DUMP+++++\n");

	print_details(list);

	for (int i = 0; i < list->dlls->num_elements; ++i) {
		dll_t *dll = ((dll_t *)al_get(list->dlls, i));

		if (dll->num_nodes > 0) {
			printf("Blocks with %lu bytes - %d free block(s) :",
				   dll->block_size, dll->num_nodes);

			dll_node_t *node = dll->head;
			do {
			#ifdef DEBUG_MODE
				printf(" 0x%lx (fi = %d)",
					   ((free_block_t *)node->data)->start_addr,
					   ((free_block_t *)node->data)->fragment_index);
			#else
				printf(" 0x%lx", ((free_block_t *)node->data)->start_addr);
			#endif
				node = node->next;
			} while (node != dll->head);
			printf("\n");
		}
	}
	printf("Allocated blocks :");
	for (int i = 0; i < list->allocd_blocks->num_elements; ++i) {
		block_t *b = ((block_t *)al_get(list->allocd_blocks, i));
	#ifdef DEBUG_MODE
		printf(" (0x%lx - %lu, fi = %d)", b->start_addr, b->block_size,
			   b->fragment_index);
	#else
		printf(" (0x%lx - %lu)", b->start_addr, b->block_size);
	#endif
	}
	printf("\n");

	printf("-----DUMP-----\n");
}
