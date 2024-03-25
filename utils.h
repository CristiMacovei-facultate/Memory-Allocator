#ifndef UTILS_H_GUARD
#define UTILS_H_GUARD

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "structs.h"

char *strdup(char *str);

size_t atolx(char *str);

int starts_with(char *cmd, char *string);

int dll_greater_equal(const void *target_size, const void *a);

int block_address_less_equal(const void *addr, const void *block);

int block_address_greater(const void *addr, const void *block);

int is_contiguous(sfl_t *list, block_t *target_block, int target_block_idx,
				  size_t target_addr);

void insert_new_shard(sfl_t *list, size_t shard_addr,
					  size_t shard_size, int fragment_index);

#endif
