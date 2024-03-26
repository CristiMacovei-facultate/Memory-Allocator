#ifdef DEBUG_MODE
#include <stdio.h>
#endif

#include "../structs.h"

void handle_destroy(sfl_t **ptr_list)
{
	sfl_t *list = *ptr_list;

	// free all elements of dll
	for (int i = 0; i < list->dlls->num_elements; ++i) {
		dll_t *dll = ((dll_t *)al_get(list->dlls, i));
		dll_node_t *head = dll->head;

		if (dll->num_nodes == 0)
			continue;

		dll_node_t *node = head;
		do {
			dll_node_t *tmp = node;
			// printf("Trying to free node addr = 0x%lx\n", node->start_addr);
			node = node->next;

			#ifdef DEBUG_MODE
			printf("Will free node of addr 0x%lx\n",
				   ((free_block_t *)tmp->data)->start_addr);
			#endif

			free(tmp->data);
			free(tmp);
		} while (node != head);
	}

	// free data contained in allocd blocks
	for (int i = 0; i < list->allocd_blocks->num_elements; ++i) {
		block_t *b = ((block_t *)al_get(list->allocd_blocks, i));
		free(b->data);
	}

	al_free(list->dlls);
	al_free(list->allocd_blocks);

	if (list->type == 1)
		al_free(list->fragmentations);

	free(list);

	*ptr_list = NULL;
}
