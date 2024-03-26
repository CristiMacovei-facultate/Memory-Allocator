// Nicolae-Cristian MACOVEI, Anul I, Grupa 312CAb

#ifdef DEBUG_MODE
#include <stdio.h>
#endif

#include "../structs.h"
#include "../cmd_utils.h"

void handle_init(char *cmd, sfl_t **ptr_list)
{	size_t start_addr = 0;
	int num_lists = 8;
	size_t bytes_per_list = 1024;
	int rec_type = 0;

	parse_init_args(cmd, &start_addr, &num_lists, &bytes_per_list, &rec_type);

	sfl_t *list = malloc(sizeof(sfl_t));
	list->start_addr = start_addr;
	list->type = rec_type;

	list->dlls = al_create(num_lists, sizeof(dll_t));
	list->allocd_blocks = al_create(1, sizeof(block_t));
	if (list->type) {
		list->fragmentations = al_create(2, sizeof(fragm_data_t));

		fragm_data_t *fd_zero = malloc(sizeof(fragm_data_t));
		fd_zero->parent_fragm = 0;
		al_insert(list->fragmentations, 0, fd_zero);
		free(fd_zero);
	}

	list->total_mem = bytes_per_list * num_lists;

	list->num_allocs = 0;
	list->num_fragmentations = 0;
	list->num_frees = 0;
	list->total_allocd = 0;

	size_t addr = list->start_addr;
	for (int i = 0; i < num_lists; ++i) {
		size_t block_size = ((size_t)8 << i);
		int num_blocks = bytes_per_list / block_size;

	#ifdef DEBUG_MODE
		printf("[DEBUG] Will alloc list of %d blocks with %lu size each\n",
			   num_blocks, block_size);
	#endif

		dll_t *tmp = dll_create_empty(block_size);
		al_insert(list->dlls, i, tmp);
		free(tmp);

		dll_t *dll = ((dll_t *)al_get(list->dlls, i));

	#ifdef DEBUG_MODE
		printf("[DEBUG] set to %lu\n", dll->block_size);
	#endif

		for (int j = 0; j < num_blocks; ++j) {
			dll_node_t *new = malloc(sizeof(dll_node_t));
			free_block_t *new_block = malloc(sizeof(free_block_t));
			new->data = new_block;
			new_block->start_addr = addr;
			new_block->fragment_index = 0; // not a fragment of anything yet
			addr += block_size;

			dll_insert_last(dll, new);
		}
	}

#ifdef DEBUG_MODE
	printf("Finished alloc, num lists: %d\n", list->dlls->num_elements);
#endif

	*ptr_list = list;
}
