// todo add debug mode

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "dll.h"
#include "arraylist.h"
#include "structs.h"

#define CMD_LINE 600

char *strdup(char *str)
{
	char *ans = malloc(strlen(str) + 1);
	strcpy(ans, str);

	return ans;
}

size_t atox(char *str)
{
#ifdef DEBUG_MODE
	printf("Passing %s into this shitty function\n", str);
#endif 

	size_t ans = 0;
	size_t n = strlen(str);
	for (size_t i = 0; i < n; ++i)
	{
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
	for (int i = 0; i < n; ++i)
	{
		if (tolower(cmd[i]) != tolower(string[i]))
			return 0;
	}

	return 1;
}

int dll_greater_equal(const void* target_size, const void *a) {
	return ((dll_t*)a)->block_size >= (*(size_t*)target_size);
}

int block_address_less_equal(const void *addr, const void* block) {
	return (*(size_t*)addr) >= ((block_t*)block)->start_addr;
}

int block_address_greater(const void *addr, const void* block) {
	return ((block_t*)block)->start_addr > (*(size_t*)addr);
}

void insert_new_shard(sfl_t *list, size_t shard_addr, size_t shard_size, int fragment_index) {
	dll_node_t *shard = malloc(sizeof(dll_node_t));
	shard->start_addr = shard_addr;
	shard->fragment_index = fragment_index;
	shard->next = NULL;
	shard->prev = NULL;

	int shard_dll_idx = al_first_if(list->dlls, &shard_size, dll_greater_equal);
	int exact_match = ((dll_t*)al_get(list->dlls, shard_dll_idx))->block_size == shard_size;

#ifdef DEBUG_MODE
	printf("Shard of size %lu (fi = %d) will be inserted in list %d (size = %lu, exact = %d)\n", shard_size, fragment_index, shard_dll_idx, ((dll_t*)al_get(list->dlls, shard_dll_idx))->block_size , exact_match);
#endif

	if (exact_match) {
		dll_t *shard_dll = al_get(list->dlls, shard_dll_idx);
		// dll_insert_last(shard_dll, shard);
		dll_insert_before_first_if(shard_dll, shard); // todo make this ADT
	}
	else {
		dll_t *shard_dll = dll_create_from_node(shard_size, shard);
		// printf("Before on %d dlls\n", list->dlls->num_elements);
		al_insert(list->dlls, shard_dll_idx, shard_dll);
		free(shard_dll);

#ifdef DEBUG_MODE
		printf("Now on %d dlls out of %lu\n", list->dlls->num_elements, list->dlls->capacity);
#endif
	}
}


void repair_fragmentation(sfl_t *list, int fragment_index, size_t block_addr, size_t block_size) { 
	if (fragment_index == 0) {
	#ifdef DEBUG_MODE
		printf("[DEBUG] Fixed everything until root fragmentation, adding new shard and exiting repair\n");
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

	int shard_dll_idx = al_first_if(list->dlls, &free_shard_size, dll_greater_equal);
	dll_t *free_shard_dll = (dll_t*)al_get(list->dlls, shard_dll_idx);

	if (free_shard_dll->num_nodes == 0) {
	#ifdef DEBUG_MODE
		printf("Shard dll not found, adding new shard and exiting repair\n");
	#endif
		insert_new_shard(list, block_addr, block_size, fragment_index);
		return;
	}

#ifdef DEBUG_MODE
	printf("Found shard dll #%d, looking up addr 0x%lx\n", shard_dll_idx, free_shard_addr);
#endif

	dll_node_t *shard = dll_find_first_if(free_shard_dll, free_shard_addr);
	if (shard == NULL || shard->start_addr != free_shard_addr) {
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
	free(shard);
	
	// attempt to recursively fix parent fragmentation
	repair_fragmentation(list, fd->parent_fragm, fd->shards[0].addr, block_size + free_shard_size);
}


void handle_init(char *cmd, sfl_t **ptr_list) {
	char sep[] = " ";
	char *p = strtok(cmd, sep);

	size_t start_addr = 0;
	int num_lists = 8;
	size_t bytes_per_list = 1024;
	int rec_type = 0;

	int i = 0; 
	while (i < 5 && p) {
		// start address 
		if (i == 1) {
			size_t tmp_start_addr = atox(p + 2); 
			if (tmp_start_addr) {
				start_addr = tmp_start_addr;
			}
		}
		// number of lists
		else if (i == 2) {
			int tmp_num_lists = atoi(p);
			if (tmp_num_lists) {
				num_lists = tmp_num_lists;
			}
		}
		// bytes per list
		else if (i == 3) {
			size_t tmp_bpl = atol(p);
			if (tmp_bpl) {
				bytes_per_list = tmp_bpl;
			}
		}
		// type of reconstruction
		else if (i == 4) {
			size_t tmp_type = atol(p);
			if (tmp_type) {
				rec_type = tmp_type;
			}
		}

		++i;
		p = strtok(NULL, sep);
	}

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
		printf("[DEBUG] Will alloc list of %d blocks with %lu size each\n", num_blocks, block_size);
	#endif 

		dll_t *tmp = dll_create_empty(block_size);
		al_insert(list->dlls, i, tmp);
		free(tmp);

	#ifdef DEBUG_MODE
		printf("[DEBUG] set to %lu\n", ((dll_t *)al_get(list->dlls, i))->block_size);
	#endif

		dll_t *dll = ((dll_t *)al_get(list->dlls, i));
		for (int j = 0; j < num_blocks; ++j) {
			// printf("[d] Alloc block %d on addr %lu\n", i, addr);
			dll_node_t *new = malloc(sizeof(dll_node_t));
		
			new->start_addr = addr;
			new->fragment_index = 0; // not a fragment of anything yet 
			addr += block_size;

			dll_insert_last(dll, new);
		}
	}

#ifdef DEBUG_MODE
	printf("Finished alloc, num lists: %d\n", list->dlls->num_elements);
#endif 

	*ptr_list = list;
}

void handle_destroy(sfl_t **ptr_list) {  
	sfl_t *list = *ptr_list;

	// free all elements of dll
	for (int i = 0; i < list->dlls->num_elements; ++i) {
		dll_t *dll = ((dll_t *)al_get(list->dlls, i));
		dll_node_t *head = dll->head;

		if (dll->num_nodes == 0) {
			continue;
		}
		
		dll_node_t *node = head;
		do {
			dll_node_t *tmp = node;
			// printf("Trying to free node addr = 0x%lx\n", node->start_addr);
			node = node->next;

			free(tmp);
		} while (node != head);
	}

	// free data contained in allocd blocks
	for (int i = 0; i < list->allocd_blocks->num_elements; ++i) {
		block_t *b = ((block_t*)al_get(list->allocd_blocks, i));
		free(b->data);
	}

	al_free(list->dlls);
	al_free(list->allocd_blocks);

	if (list->type == 1) {
		al_free(list->fragmentations);
	}
	
	free(list);

	*ptr_list = NULL;
}


void handle_print(sfl_t *list) {
	printf("+++++DUMP+++++\n");

	printf("Total memory: %lu bytes\n", list->total_mem);
	printf("Total allocated memory: %lu bytes\n", list->total_allocd);
	printf("Total free memory: %lu bytes\n", list->total_mem - list->total_allocd);

	int num_free_blocks = 0;
	for (int i = 0; i < list->dlls->num_elements; ++i) {
		dll_t* dll = al_get(list->dlls, i);
		num_free_blocks += dll->num_nodes;
	}
	printf("Free blocks: %d\n", num_free_blocks);

	printf("Number of allocated blocks: %d\n", list->allocd_blocks->num_elements);
	printf("Number of malloc calls: %d\n", list->num_allocs);
	printf("Number of fragmentations: %d\n", list->num_fragmentations);
	printf("Number of free calls: %d\n", list->num_frees);

	for (int i = 0; i < list->dlls->num_elements; ++i) {
		dll_t *dll = ((dll_t *)al_get(list->dlls, i));

		if (dll->num_nodes > 0) {
			printf("Blocks with %lu bytes - %d free block(s) :", dll->block_size, dll->num_nodes);

			dll_node_t *node = dll->head;
			do {
			#ifdef DEBUG_MODE
				printf(" 0x%lx (fi = %d)", node->start_addr, node->fragment_index);
			#else
				printf(" 0x%lx", node->start_addr);
			#endif 
				node = node->next;
			} while (node != dll->head);
			printf("\n");
		}
	}
	printf("Allocated blocks :");
	for (int i = 0; i < list->allocd_blocks->num_elements; ++i) {
		block_t *b = ((block_t*)al_get(list->allocd_blocks, i));
	#ifdef DEBUG_MODE
		printf(" (0x%lx - %lu, fi = %d)", b->start_addr, b->block_size, b->fragment_index);
	#else
		printf(" (0x%lx - %lu)", b->start_addr, b->block_size);
	#endif
	}
	printf("\n");

	printf("-----DUMP-----\n");
}

void handle_malloc(char *cmd, sfl_t *list) {
	if (!list) {
		printf("Heap was not initialised. You are a massive idiot :)\n");
		return;
	}

	char sep[] = " ";
	char *p = strtok(cmd, sep);

	size_t requested = 0;

	int arg_index = 0; 
	while (arg_index < 2 && p) {
	#ifdef DEBUG_MODE
		printf("Malloc arg [%d]: '%s'\n", arg_index, p);
	#endif
		
		// start address 
		if (arg_index == 1) {
			size_t tmp_requested = atol(p);
			if (tmp_requested) {
				requested = tmp_requested;
			}
		}

		++arg_index;
		p = strtok(NULL, sep);
	}

	if (!requested) {
		printf("Cannot alloc 0 bytes\n");
		return;
	}

#ifdef DEBUG_MODE
	printf("[DEBUG] Have to alloc %lu bytes\n", requested);
#endif

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

		if (dll->block_size < requested) {
			continue;
		}

		// alloc head of dll
		dll_node_t *mallocd_node = dll_pop_first(dll);

	#ifdef DEBUG_MODE
		printf("Will malloc on addr %lu\n", mallocd_node->start_addr);
		printf("Will break block of size %lu into %lu and %lu\n", dll->block_size, requested, dll->block_size - requested);
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
			insert_new_shard(list, shard_addr, shard_size, list->num_fragmentations);
		}
	#ifdef DEBUG_MODE
		else {
			printf("[DEBUG] Shard size zero, skipping.\n");
		}
	#endif

		int block_idx = al_first_if(list->allocd_blocks, &(new_block->start_addr), block_address_greater);
		al_insert(list->allocd_blocks, block_idx, new_block);

		free(mallocd_node);
		free(new_block);

		return;
	}

	printf("Out of memory\n");
}

void handle_read(char *cmd, sfl_t **ptr_list) {
	sfl_t *list = *ptr_list;
	if (!list) {
		printf("Heap was not initialised. You are a massive idiot :)\n");
		return;
	}

	char sep[] = " ";
	char *p = strtok(cmd, sep);

	size_t addr = 0;
	size_t num_bytes = 0;

	int arg_index = 0; 
	while (arg_index < 4 && p) {
		// printf("Arg %d: '%s'\n", i, p);
		
		// start address 
		if (arg_index == 1) {
			size_t tmp_addr = atox(p + 2);
			if (tmp_addr) {
				addr = tmp_addr;
			}
		}
		else if (arg_index == 2) {
			size_t tmp_num_bytes = atoll(p);
			if (tmp_num_bytes) {
				num_bytes = tmp_num_bytes;
			}
		}

		++arg_index;
		p = strtok(NULL, sep);
	}

#ifdef DEBUG_MODE
	printf("[DEBUG] Reading %lu bytes from address %lu\n", num_bytes, addr);
#endif

	int target_block_idx = al_last_if(list->allocd_blocks, &addr, block_address_less_equal);
	block_t *target_block = al_get(list->allocd_blocks, target_block_idx);
	int exact_match = target_block->start_addr <= addr && target_block->start_addr + target_block->block_size > addr;

	if (!exact_match) {
	#ifdef DEBUG_MODE
		printf("Segmentation Fault (not alloc'd). Esti prost facut gramada\n");
	#else
		printf("Segmentation fault (core dumped)\n");
	#endif 
		handle_print(list);
		handle_destroy(ptr_list);
		return;
	}

	size_t contiguous_until = target_block->start_addr + target_block->block_size;
#ifdef DEBUG_MODE
	printf("[DEBUG] Initially contig until %lu\n", contiguous_until);
#endif
	for (int i = target_block_idx + 1; i < list->allocd_blocks->num_elements; ++i) {
		block_t *next_block = al_get(list->allocd_blocks, i);
		if (next_block->start_addr == contiguous_until) {
			contiguous_until = next_block->start_addr + next_block->block_size;
		#ifdef DEBUG_MODE
			printf("contig until %lu\n", contiguous_until);
		#endif
		}
		else {
			break;
		}
	}
	if (contiguous_until < addr + num_bytes) {
	#ifdef DEBUG_MODE
		printf("Segmentation Fault (not all bytes alloc'd - contig. until %lu, needed %lu). Esti prost facut gramada\n", contiguous_until, addr + num_bytes);
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
		
		uint8_t *ptr_byte = ((uint8_t*)target_block->data + index);
		printf("%c", *ptr_byte);
		
		++index;
	}
	printf("\n");
}

void handle_free(char *cmd, sfl_t *list) {
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
			size_t tmp_addr = atox(p + 2);
			if (tmp_addr) {
				addr = tmp_addr;
			}
		}

		++arg_index;
		p = strtok(NULL, sep);
	}

#ifdef DEBUG_MODE
	printf("[DEBUG] Freeing address 0x%lx (%ld)\n", addr, addr);
#endif

	int block_idx = al_first_if(list->allocd_blocks, &addr, block_address_greater) - 1;

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

	if (list->type) {
		repair_fragmentation(list, fragm_idx, new_addr, new_size);
	}
	else {
		insert_new_shard(list, new_addr, new_size, fragm_idx);
	}

	++(list->num_frees);
	list->total_allocd -= new_size;
}

void handle_write(char *cmd, sfl_t **ptr_list) {
	sfl_t *list = *ptr_list;
	if (!list) {
		printf("Heap was not initialised. You are a massive idiot :)\n");
		return;
	}

	// char sep[] = " ";
	// char *p = strtok(cmd, sep);

	size_t addr = 0;
	char *data = NULL;
	size_t num_bytes = 0;

	strcat(cmd, " ");
	size_t n = strlen(cmd);

	int arg_index = 0;
	size_t prev_space = -1;
	int num_quotes = 0;
	for (size_t i = 0; i < n; ++i) {
		if (cmd[i] == '"') {
			num_quotes = 1 - num_quotes;
		}

		if (cmd[i] == ' ' && num_quotes == 0) {
			if (prev_space != (i - 1)) {
				cmd[i] = '\0';

				char *arg = (cmd + prev_space + 1);

				// start address 
				if (arg_index == 1) {
					size_t tmp_addr = atox(arg + 2);
					if (tmp_addr) {
						addr = tmp_addr;
					}
				}
				else if (arg_index == 2) {
					if (arg[0] == '"') {
						arg[strlen(arg) - 1] = '\0';
						data = strdup(arg + 1);
						arg[strlen(arg)] = ' ';
					}
					else {
						data = strdup(arg);
					}
				}
				else if (arg_index == 3) {
					size_t tmp_num_bytes = atoll(arg);
					if (tmp_num_bytes) {
						num_bytes = tmp_num_bytes;
					}
				}

				cmd[i] = ' ';
				++arg_index;
			}

			prev_space = i;
		}
	}

	if (data == NULL) {
		return;
	}

	if (num_bytes > strlen(data)) {
		num_bytes = strlen(data);
	}

#ifdef DEBUG_MODE
	printf("[DEBUG] Will write %lu bytes of '%s' into addr %lx\n", num_bytes, data, addr);
#endif 

	int target_block_idx = al_last_if(list->allocd_blocks, &addr, block_address_less_equal);
	block_t *target_block = al_get(list->allocd_blocks, target_block_idx);
	int exact_match = target_block->start_addr <= addr && target_block->start_addr + target_block->block_size > addr;

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

	size_t contiguous_until = target_block->start_addr + target_block->block_size;
#ifdef DEBUG_MODE
	printf("[DEBUG] Initially contig until %lu\n", contiguous_until);
#endif 
	for (int i = target_block_idx + 1; i < list->allocd_blocks->num_elements; ++i) {
		block_t *next_block = al_get(list->allocd_blocks, i);
		if (next_block->start_addr == contiguous_until) {
			contiguous_until = next_block->start_addr + next_block->block_size;
		#ifdef DEBUG_MODE
			printf("contig until %lu\n", contiguous_until);
		#endif
		}
		else {
			break;
		}
	}
	if (contiguous_until < addr + num_bytes) {
		free(data);

	#ifdef DEBUG_MODE
		printf("Segmentation Fault (not all bytes alloc'd - contig. until %lu, needed %lu). Esti prost facut gramada\n", contiguous_until, addr + num_bytes);
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
		
		uint8_t *ptr_byte = ((uint8_t*)target_block->data + index);
		*ptr_byte = (uint8_t)data[i];

		++index;
	#ifdef DEBUG_MODE
		printf("[DEBUG] Wrote char %c on index %d\n", data[i], index);
	#endif 
	}

	free(data);
}

int main() {
	char cmd[CMD_LINE];

	sfl_t *list = NULL;

	while (!feof(stdin)) {
		fgets(cmd, CMD_LINE, stdin);

		if (cmd[strlen(cmd) - 1] == '\n') {
			cmd[strlen(cmd) - 1] = '\0';
		}

		if (starts_with(cmd, "INIT_HEAP")) {
			handle_init(cmd, &list);
		}
		#ifdef DEBUG_MODE
		else if (starts_with(cmd, "PRINT")) {
			handle_print(list);
		}
		#endif
		else if (starts_with(cmd, "DUMP_MEMORY")) {
			handle_print(list);
		}
		else if (starts_with(cmd, "MALLOC")) {
			handle_malloc(cmd, list);
		}
		else if (starts_with(cmd, "READ")) {
			handle_read(cmd, &list);

			if (!list) {
				return 0;
			}
		}
		else if (starts_with(cmd, "WRITE")) {
			handle_write(cmd, &list);

			if (!list) {
				return 0;
			}
		}
		else if (starts_with(cmd, "FREE")) {
			handle_free(cmd, list);
		}
		else if (starts_with(cmd, "DESTROY_HEAP")) {
			handle_destroy(&list);
			return 0;
		}
	}

	return 0;
}