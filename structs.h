#ifndef STRUCTS_H_GUARD
#define STRUCTS_H_GUARD

#include <stdlib.h>

#include "dll.h"
#include "arraylist.h"

typedef struct block {
	size_t block_size;
	size_t start_addr;

	int fragment_index;
	void *data;
} block_t;

typedef struct free_block {
	size_t start_addr;
	int fragment_index;
} free_block_t;

typedef struct sfl {
	size_t start_addr;
	unsigned char type;

	arraylist_t *dlls; // arraylist of dll_t
	arraylist_t *allocd_blocks; // array list of block_t
	arraylist_t *fragmentations; // array list of fragm_data_t

	// for memdump
	size_t total_mem;
	size_t total_allocd;
	int num_allocs;
	int num_frees;
	int num_fragmentations;
} sfl_t;

// data regarding a specific fragmentation
typedef struct fragment_data {
	int parent_fragm; // to recursively attempt to fix parent

	struct shard_data {
		size_t addr;
		size_t size;
	} shards[2];
} fragm_data_t;

#endif
