#include <stdio.h>

#include "../structs.h"
#include "../utils.h"
#include "../cmd_utils.h"

int malloc_internal(sfl_t *list, size_t requested)
{
	for (int i = 0; i < list->dlls->num_elements; ++i) {
	#ifdef DEBUG_MODE
		printf("[DEBUG] Searching on list %d\n", i);
	#endif
		dll_t *dll = al_get(list->dlls, i);

		if (dll->num_nodes == 0) {
		#ifdef DEBUG_MODE
			printf("[DEBUG] Skipping list %d, zero blocks remaining\n", i);
		#endif

			continue;
		}

		if (dll->block_size < requested)
			continue;

		// alloc head of dll
		dll_node_t *mallocd_node = dll_pop_first(dll);

	#ifdef DEBUG_MODE
		printf("Will malloc on addr %lu\n", mallocd_node->start_addr);
		printf("Will break block of size %lu into %lu and %lu\n",
			   dll->block_size, requested, dll->block_size - requested);
	#endif

		// append new block to arraylist for alloc'd blocks
		// it's done in a way so that the list stays sorted in increasing order
		block_t *new_block = malloc(sizeof(block_t));
		new_block->block_size = requested;
		new_block->start_addr = mallocd_node->start_addr;
		new_block->data = calloc(requested, sizeof(uint8_t));
		new_block->fragment_index = mallocd_node->fragment_index; // initially

		(list->num_allocs)++;
		(list->total_allocd) += requested;
		size_t shard_addr = mallocd_node->start_addr + requested;
		size_t shard_size = dll->block_size - requested;

		if (shard_size > 0) {
			list->num_fragmentations++;

			if (list->type == 1) {
				fragm_data_t *fd = malloc(sizeof(fragm_data_t));
				fd->shards[0].size = new_block->block_size;
				fd->shards[0].addr = new_block->start_addr;
				fd->shards[1].size = shard_size;
				fd->shards[1].addr = shard_addr;
				fd->parent_fragm = mallocd_node->fragment_index;

				al_insert(list->fragmentations, list->num_fragmentations, fd);
				free(fd);
			}

			new_block->fragment_index = list->num_fragmentations;
			insert_new_shard(list, shard_addr, shard_size,
							 list->num_fragmentations);
		}
	#ifdef DEBUG_MODE
		else
			printf("[DEBUG] Shard size zero, skipping.\n");
	#endif

		int block_idx = al_first_if(list->allocd_blocks,
									&new_block->start_addr,
									block_address_greater);
		al_insert(list->allocd_blocks, block_idx, new_block);

		free(mallocd_node);
		free(new_block);

		return 1;
	}

	return 0;
}

void handle_malloc(char *cmd, sfl_t *list)
{
	if (!list) {
		printf("Heap was not initialised.\n");
		return;
	}

	size_t requested = 0;
	parse_malloc_args(cmd, &requested);

	if (!requested) {
		printf("Cannot alloc 0 bytes\n");
		return;
	}

#ifdef DEBUG_MODE
	printf("[DEBUG] Have to alloc %lu bytes\n", requested);
#endif

	int malloc_success = malloc_internal(list, requested);

	if (!malloc_success)
		printf("Out of memory\n");
}
