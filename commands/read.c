#include <stdio.h>

#include "../structs.h"
#include "../utils.h"
#include "../cmd_utils.h"

#include "print.h"
#include "destroy.h"

void handle_read(char *cmd, sfl_t **ptr_list)
{
	sfl_t *list = *ptr_list;
	if (!list) {
		printf("Heap was not initialised.\n");
		return;
	}

	size_t addr = 0;
	size_t num_bytes = 0;
	parse_read_args(cmd, &addr, &num_bytes);

#ifdef DEBUG_MODE
	printf("[DEBUG] Reading %lu bytes from address %lu\n", num_bytes, addr);
#endif

	int target_block_idx = al_last_if(list->allocd_blocks, &addr,
										block_address_less_equal);
	block_t *target_block = al_get(list->allocd_blocks, target_block_idx);
	size_t tmp_end_addr = target_block->start_addr + target_block->block_size;
	int exact_match = target_block->start_addr <= addr &&
						tmp_end_addr > addr;

	if (!exact_match) {
	#ifdef DEBUG_MODE
		printf("Segmentation Fault (not alloc'd).\n");
	#else
		printf("Segmentation fault (core dumped)\n");
	#endif
		handle_print(list);
		handle_destroy(ptr_list);
		return;
	}

	size_t contig_until = target_block->start_addr + target_block->block_size;
#ifdef DEBUG_MODE
	printf("[DEBUG] Initially contig until %lu\n", contig_until);
#endif
	int num_blocks_allocd = list->allocd_blocks->num_elements;
	for (int i = target_block_idx + 1; i < num_blocks_allocd; ++i) {
		block_t *next_block = al_get(list->allocd_blocks, i);
		if (next_block->start_addr == contig_until) {
			contig_until = next_block->start_addr + next_block->block_size;
		#ifdef DEBUG_MODE
			printf("contig until %lu\n", contig_until);
		#endif
		} else {
			break;
		}
	}
	if (contig_until < addr + num_bytes) {
	#ifdef DEBUG_MODE
		printf("Segmentation Fault (not all bytes alloc'd - contig. until");
		printf("%lu, needed %lu)", contig_until, addr + num_bytes);
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
		printf("%c", *ptr_byte);

		++index;
	}
	printf("\n");
}
