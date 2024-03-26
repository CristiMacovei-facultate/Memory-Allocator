// Nicolae-Cristian MACOVEI, Anul I, Grupa 312CAb

#ifdef DEBUG_MODE
#include <stdio.h>
#endif

#include "../structs.h"
#include "../utils.h"

void repair_fragmentation(sfl_t *list, int fragment_index, size_t block_addr,
						  size_t block_size)
{
	if (fragment_index == 0) {
	#ifdef DEBUG_MODE
		printf("[DEBUG] Fixed everything until root fragmentation,");
		printf("adding new shard and exiting repair\n");
	#endif
		insert_new_shard(list, block_addr, block_size, fragment_index);
		return;
	}

	fragm_data_t *fd = al_get(list->fragmentations, fragment_index);

#ifdef DEBUG_MODE
	printf("[DEBUG] Repairing fragment %d\n", fragment_index);
#endif

	size_t free_shard_idx = fd->shards[0].addr != block_addr ? 0 : 1;
	size_t free_shard_size = fd->shards[free_shard_idx].size;
	size_t free_shard_addr = fd->shards[free_shard_idx].addr;
#ifdef DEBUG_MODE
	printf("Free shard is in list of size %lu\n", free_shard_size);
#endif

	int shard_dll_idx = al_first_if(list->dlls, &free_shard_size,
									dll_greater_equal);
	dll_t *free_shard_dll = (dll_t *)al_get(list->dlls, shard_dll_idx);

	if (free_shard_dll->num_nodes == 0) {
	#ifdef DEBUG_MODE
		printf("Shard dll not found, adding new shard and exiting repair\n");
	#endif
		insert_new_shard(list, block_addr, block_size, fragment_index);
		return;
	}

#ifdef DEBUG_MODE
	printf("Found shard dll #%d, looking up addr 0x%lx\n",
		   shard_dll_idx, free_shard_addr);
#endif

	dll_node_t *shard = dll_find_first_if(free_shard_dll, free_shard_addr);
	if (!shard ||
		((free_block_t *)shard->data)->start_addr != free_shard_addr) {
	#ifdef DEBUG_MODE
		printf("Shard not found, adding new shard and exiting repair\n");
	#endif
		insert_new_shard(list, block_addr, block_size, fragment_index);
		return;
	}

#ifdef DEBUG_MODE
	printf("Found shard, blowing it up from list\n");
#endif
	dll_erase_node(free_shard_dll, shard);
	free(shard->data);
	free(shard);

	// attempt to recursively fix parent fragmentation
	repair_fragmentation(list, fd->parent_fragm, fd->shards[0].addr,
						 block_size + free_shard_size);
}

void handle_free(char *cmd, sfl_t *list)
{
	if (!list) {
		printf("Heap was not initialised. You are a massive idiot :)\n");
		return;
	}

	char sep[] = " ";
	char *p = strtok(cmd, sep);

	size_t addr = 0;

	int arg_index = 0;
	while (arg_index < 2 && p) {
	#ifdef DEBUG_MODE
		printf("[DEBUG] Free - arg [%d]: '%s'\n", arg_index, p);
	#endif

		// start address
		if (arg_index == 1) {
			size_t tmp_addr = atolx(p + 2);
			if (tmp_addr)
				addr = tmp_addr;
		}

		++arg_index;
		p = strtok(NULL, sep);
	}

#ifdef DEBUG_MODE
	printf("[DEBUG] Freeing address 0x%lx (%ld)\n", addr, addr);
#endif

	int block_idx = al_first_if(list->allocd_blocks,
								&addr, block_address_greater) - 1;

	if (block_idx < 0) {
		printf("Invalid free\n");
		return;
	}

	block_t *block = al_get(list->allocd_blocks, block_idx);

	if (block->start_addr != addr) {
		printf("Invalid free\n");
		return;
	}

#ifdef DEBUG_MODE
	printf("[DEBUG] Freeing from index %d\n", block_idx);
#endif

	size_t new_addr = block->start_addr;
	size_t new_size = block->block_size;
	int fragm_idx = block->fragment_index;

#ifdef DEBUG_MODE
	printf("Frag index = %d\n", fragm_idx);
#endif

	free(block->data);
	al_erase(list->allocd_blocks, block_idx);

	if (list->type)
		repair_fragmentation(list, fragm_idx, new_addr, new_size);
	else
		insert_new_shard(list, new_addr, new_size, fragm_idx);

	++(list->num_frees);
	list->total_allocd -= new_size;
}
