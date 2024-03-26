#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include "structs.h"

// comparator functions
int dll_greater_equal(const void *target_size, const void *a)
{
	return ((dll_t *)a)->block_size >= (*(size_t *)target_size);
}

int block_address_less_equal(const void *addr, const void *block)
{
	return (*(size_t *)addr) >= ((block_t *)block)->start_addr;
}

int block_address_greater(const void *addr, const void *block)
{
	return ((block_t *)block)->start_addr > (*(size_t *)addr);
}

char *strdup(char *str)
{
	char *ans = malloc(strlen(str) + 1);
	strcpy(ans, str);

	return ans;
}

size_t atolx(char *str)
{
#ifdef DEBUG_MODE
	printf("Passing %s into this shitty function\n", str);
#endif

	size_t ans = 0;
	size_t n = strlen(str);
	for (size_t i = 0; i < n; ++i) {
		int digit = str[i] - '0';
		if ('a' <= tolower(str[i]) && tolower(str[i]) <= 'f')
			digit = 10 + tolower(str[i]) - 'a';
		ans = ans * 16 + digit;
	}
	return ans;
}

int starts_with(char *cmd, char *string)
{
	int n = strlen(string);
	for (int i = 0; i < n; ++i) {
		if (tolower(cmd[i]) != tolower(string[i]))
			return 0;
	}

	return 1;
}

void insert_new_shard(sfl_t *list, size_t shard_addr,
					  size_t shard_size, int fragment_index)
{
	dll_node_t *shard = malloc(sizeof(dll_node_t));
	shard->data = malloc(sizeof(free_block_t));
	((free_block_t *)shard->data)->start_addr = shard_addr;
	((free_block_t *)shard->data)->fragment_index = fragment_index;
	shard->next = NULL;
	shard->prev = NULL;

	int shard_dll_idx = al_first_if(list->dlls, &shard_size, dll_greater_equal);
	dll_t *shard_dll = (dll_t *)al_get(list->dlls, shard_dll_idx);
	int exact_match = shard_dll->block_size == shard_size;

#ifdef DEBUG_MODE
	printf("Shard of size %lu (fi = %d) will be inserted in list %d ",
		   shard_size, fragment_index, shard_dll_idx);
	printf("(size = %lu, exact = %d)\n", shard_dll->block_size, exact_match);
#endif

	if (exact_match) {
		dll_insert_before_first_if(shard_dll, shard); // todo make this ADT
	} else {
		shard_dll = dll_create_from_node(shard_size, shard);
		// printf("Before on %d dlls\n", list->dlls->num_elements);
		al_insert(list->dlls, shard_dll_idx, shard_dll);
		free(shard_dll);

#ifdef DEBUG_MODE
		printf("Now on %d dlls out of %lu\n", list->dlls->num_elements,
			   list->dlls->capacity);
#endif
	}
}

int is_contiguous(sfl_t *list, block_t *target_block, int target_block_idx,
				  size_t target_addr)
{
	size_t contig_until = target_block->start_addr + target_block->block_size;
#ifdef DEBUG_MODE
	printf("[DEBUG] Initially contig until %lu\n", contig_until);
#endif
	int num_allocd_blocks = list->allocd_blocks->num_elements;
	for (int i = target_block_idx + 1; i < num_allocd_blocks; ++i) {
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

	return contig_until < target_addr;
}
