// Nicolae-Cristian MACOVEI, Anul I, Grupa 312CAb

#include <stdio.h>

#include "../structs.h"
#include "../utils.h"
#include "../cmd_utils.h"

#include "print.h"
#include "destroy.h"

void handle_write(char *cmd, sfl_t **ptr_list)
{
	sfl_t *list = *ptr_list;
	if (!list) {
		printf("Heap was not initialised. You are a massive idiot :)\n");
		return;
	}

	size_t addr = 0;
	char *data = NULL;
	size_t num_bytes = 0;

	parse_write_args(cmd, &addr, &data, &num_bytes);

	if (!data)
		return;

	if (num_bytes > strlen(data))
		num_bytes = strlen(data);

#ifdef DEBUG_MODE
	printf("[DEBUG] Will write %lu bytes of '%s' into addr %lx\n",
		   num_bytes, data, addr);
#endif

	int target_block_idx = al_last_if(list->allocd_blocks, &addr,
									  block_address_less_equal);
	block_t *target_block = al_get(list->allocd_blocks, target_block_idx);
	size_t tmp_end_addr = target_block->start_addr + target_block->block_size;
	int exact_match = target_block->start_addr <= addr &&
					  tmp_end_addr > addr;

	if (!exact_match) {
		free(data);
	#ifdef DEBUG_MODE
		printf("Segmentation Fault (not alloc'd). Esti prost facut gramada\n");
	#else
		printf("Segmentation fault (core dumped)\n");
	#endif
		handle_print(list);
		handle_destroy(ptr_list);
		return;
	}

	int is_contig = is_contiguous(list, target_block, target_block_idx,
									 addr + num_bytes);
	if (is_contig) {
		free(data);

	#ifdef DEBUG_MODE
		printf("Segmentation Fault (not all bytes alloc'd)");
	#else
		printf("Segmentation fault (core dumped)\n");
	#endif
		handle_print(list);
		handle_destroy(ptr_list);
		return;
	}

	int index = addr - target_block->start_addr;
	for (size_t i = 0; i < num_bytes; ++i) {
		if (i + addr >= target_block->start_addr + target_block->block_size) {
			++target_block_idx;
			target_block = al_get(list->allocd_blocks, target_block_idx);
			index = 0;
		}

		unsigned char *ptr_byte = ((unsigned char *)target_block->data + index);
		*ptr_byte = (unsigned char)data[i];

		++index;
	#ifdef DEBUG_MODE
		printf("[DEBUG] Wrote char %c on index %d\n", data[i], index);
	#endif
	}

	free(data);
}
