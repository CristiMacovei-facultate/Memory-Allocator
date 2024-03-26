#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "cmd_utils.h"
#include "utils.h"
#include "dll.h"
#include "arraylist.h"
#include "structs.h"

#include "commands/print.h"
#include "commands/malloc.h"
#include "commands/read.h"
#include "commands/write.h"
#include "commands/destroy.h"

#define CMD_LINE 600

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

int main(void)
{
	char cmd[CMD_LINE];

	sfl_t *list = NULL;

	while (!feof(stdin)) {
		fgets(cmd, CMD_LINE, stdin);

		if (cmd[strlen(cmd) - 1] == '\n')
			cmd[strlen(cmd) - 1] = '\0';

		if (starts_with(cmd, "INIT_HEAP"))
			handle_init(cmd, &list);
		#ifdef DEBUG_MODE
		else if (starts_with(cmd, "PRINT"))
			handle_print(list);
		#endif
		else if (starts_with(cmd, "DUMP_MEMORY"))
			handle_print(list);
		else if (starts_with(cmd, "MALLOC"))
			handle_malloc(cmd, list);
		else if (starts_with(cmd, "READ")) {
			handle_read(cmd, &list);

			if (!list)
				return 0;
		} else if (starts_with(cmd, "WRITE")) {
			handle_write(cmd, &list);

			if (!list)
				return 0;
		} else if (starts_with(cmd, "FREE"))
			handle_free(cmd, list);
		else if (starts_with(cmd, "DESTROY_HEAP")) {
			handle_destroy(&list);
			return 0;
		}
	}

	return 0;
}
