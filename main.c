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

/*
char *strdup(char *str) {
  size_t n = strlen(str);

  char *ans = malloc(n);
  memcpy(ans, str, n);

  return ans;
}
*/

size_t atox(char *str) {
  printf("Passing %s into this shitty function\n", str);

  size_t ans = 0;
  size_t n = strlen(str);
  for (size_t i = 0; i < n; ++i) {
    int digit = str[i] - '0';
    if ('a' <= tolower(str[i]) && tolower(str[i]) <= 'f') {
      digit = 10 + tolower(str[i]) - 'a';
    }
    ans = ans * 16 + digit;
  }
  return ans;
}

int starts_with(char *cmd, char *string) {
  int n = strlen(string);
  for (int i = 0; i < n; ++i) {
    if (tolower(cmd[i]) != tolower(string[i])) {
      return 0;
    }
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

void insert_new_shard(sfl_t *list, size_t shard_addr, size_t shard_size) {
  dll_node_t *shard = malloc(sizeof(dll_node_t));
  shard->start_addr = shard_addr;
  shard->next = NULL;
  shard->prev = NULL;

  int shard_dll_idx = al_first_if(list->dlls, &shard_size, dll_greater_equal);
  int exact_match = ((dll_t*)al_get(list->dlls, shard_dll_idx))->block_size == shard_size;
  printf("Shard of size %lu will be inserted in list %d (size = %lu, exact = %d)\n", shard_size, shard_dll_idx, ((dll_t*)al_get(list->dlls, shard_dll_idx))->block_size , exact_match);

  if (exact_match) {
    dll_t *shard_dll = al_get(list->dlls, shard_dll_idx);
    dll_insert_last(shard_dll, shard);
  }
  else {
    dll_t *shard_dll = dll_create_from_node(shard_size, shard);
    // printf("Before on %d dlls\n", list->dlls->num_elements);
    al_insert(list->dlls, shard_dll_idx, shard_dll);
    free(shard_dll);
    printf("Now on %d dlls out of %lu\n", list->dlls->num_elements, list->dlls->capacity);
  }
}

void handle_init(char *cmd, sfl_t **ptr_list) {
  char sep[] = " ";
  char *p = strtok(cmd, sep);

  size_t start_addr = 0;
  int num_lists = 8;
  size_t bytes_per_list = 1024;

  int i = 0; 
  while (i < 4 && p) {
    // start address 
    if (i == 1) {
      size_t tmp_start_addr = atol(p); 
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

    ++i;
    p = strtok(NULL, sep);
  }

  sfl_t *list = malloc(sizeof(sfl_t));
  list->start_addr = start_addr;
  list->type = 0; //todo change this in the future
  
  list->dlls = al_create(num_lists, sizeof(dll_t));
  list->allocd_blocks = al_create(1, sizeof(block_t));

  list->total_mem = bytes_per_list * num_lists;

  size_t addr = list->start_addr;
  for (int i = 0; i < num_lists; ++i) {
    size_t block_size = ((size_t)8 << i);
    int num_blocks = bytes_per_list / block_size;
    // printf("Will alloc list of %lu blocks with %lu size each\n", num_blocks, block_size);

    // printf("block size = %lu\n", block_size);
    dll_t *tmp = dll_create_empty(block_size);
    al_insert(list->dlls, i, tmp);
    free(tmp);
    // printf("set to %lu\n", ((dll_t *)al_get(list->dlls, i))->block_size);

    dll_t *dll = ((dll_t *)al_get(list->dlls, i));
    for (int j = 0; j < num_blocks; ++j) {
      // printf("[d] Alloc block %d on addr %lu\n", i, addr);
      dll_node_t *new = malloc(sizeof(dll_node_t));
    
      new->start_addr = addr;
      addr += block_size;

      dll_insert_last(dll, new);
    }
  }

  printf("Finished alloc, num lists: %d\n", list->dlls->num_elements);

  *ptr_list = list;
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
  printf("Number of free blocks: %d\n", num_free_blocks);

  printf("Number of allocated blocks: %d\n", list->allocd_blocks->num_elements);
  printf("Number of malloc calls: %d\n", list->num_allocs);
  printf("Number of fragmentations: %d\n", list->num_fragmentations);
  printf("Number of free calls: %d\n", list->num_frees);

  for (int i = 0; i < list->dlls->num_elements; ++i) {
    dll_t *dll = ((dll_t *)al_get(list->dlls, i));

    printf("Now printing list with %lu size (%d nodes)\n", dll->block_size, dll->num_nodes);

    if (dll->num_nodes > 0) {
      printf("Blocks with %lu bytes: %d free block(s) - ", dll->block_size, dll->num_nodes);

      dll_node_t *node = dll->head;
      do {
        printf("%lu ", node->start_addr);
        node = node->next;
      } while (node != dll->head);
      printf("\n");
    }
  }
  printf("Allocated blocks: ");
  for (int i = 0; i < list->allocd_blocks->num_elements; ++i) {
    block_t *b = ((block_t*)al_get(list->allocd_blocks, i));
    printf("(%lu - %lu) ", b->start_addr, b->block_size);
  }
  printf("\n");

  printf("-----DUMP-----\n");
}

void handle_malloc(char *cmd, sfl_t *list) {
  if (!list) {
    fprintf(stderr, "Heap was not initialised. You are a massive idiot :)\n");
    return;
  }

  char sep[] = " ";
  char *p = strtok(cmd, sep);

  size_t requested = 0;

  int arg_index = 0; 
  while (arg_index < 4 && p) {
    // printf("Arg %d: '%s'\n", i, p);
    
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
    fprintf(stderr, "Cannot alloc 0 bytes\n");
    return;
  }

  // printf("[d] Have to alloc %lu bytes\n", requested);
  for (int i = 0; i < list->dlls->num_elements; ++i) {
    // printf("[d] Searching on list %d\n", i);
    dll_t *dll = al_get(list->dlls, i);

    if (dll->num_nodes == 0) {
      // fprintf(stderr, "[WARN] Skipping list %d, zero blocks remaining\n", i);
      continue;
    }

    if (dll->block_size < requested) {
      continue;
    }

    // alloc head of dll
    dll_node_t *mallocd_node = dll_pop_first(dll);
    
    // dll->head = dll->head->next;
    // dll->num_nodes--;

    printf("Will malloc on addr %lu\n", mallocd_node->start_addr);
    printf("Will break block of size %lu into %lu and %lu\n", dll->block_size, requested, dll->block_size - requested);

    // append new block to arraylist for alloc'd blocks
    // it's done in a way so that the list stays sorted in increasing order
    block_t *new_block = malloc(sizeof(block_t));
    new_block->block_size = requested;
    new_block->start_addr = mallocd_node->start_addr;
    new_block->data = calloc(requested, sizeof(uint8_t));
    (list->num_allocs)++;
    (list->total_allocd) += requested;

    int block_idx = al_first_if(list->allocd_blocks, &(new_block->start_addr), block_address_greater);
    al_insert(list->allocd_blocks, block_idx, new_block);
    free(new_block);

    size_t shard_addr = mallocd_node->start_addr + requested;
    size_t shard_size = dll->block_size - requested;
    
    free(mallocd_node);

    if (shard_size > 0) {
      list->num_fragmentations++;
      insert_new_shard(list, shard_addr, shard_size);
    }
    else {
      fprintf(stderr, "Shard size zero, skipping.\n");
    }

    return;
  }

  fprintf(stderr, "Out of memory.\n");
}

void handle_read(char *cmd, sfl_t *list) {
  if (!list) {
    fprintf(stderr, "Heap was not initialised. You are a massive idiot :)\n");
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

  printf("Reading %lu bytes from address %lu\n", num_bytes, addr);

  int target_block_idx = al_last_if(list->allocd_blocks, &addr, block_address_less_equal);
  block_t *target_block = al_get(list->allocd_blocks, target_block_idx);
  int exact_match = target_block->start_addr <= addr && target_block->start_addr + target_block->block_size > addr;

  if (!exact_match) {
    fprintf(stderr, "Segmentation Fault (not alloc'd). Esti prost facut gramada\n");
    return;
  }

  size_t contiguous_until = target_block->start_addr + target_block->block_size;
  printf("Initially contig until %lu\n", contiguous_until);
  for (int i = target_block_idx + 1; i < list->allocd_blocks->num_elements; ++i) {
    block_t *next_block = al_get(list->allocd_blocks, i);
    if (next_block->start_addr == contiguous_until) {
      contiguous_until = next_block->start_addr + next_block->block_size;
      printf("contig until %lu\n", contiguous_until);
    }
    else {
      break;
    }
  }
  if (contiguous_until < addr + num_bytes) {
    fprintf(stderr, "Segmentation Fault (not all bytes alloc'd - contig. until %lu, needed %lu). Esti prost facut gramada\n", contiguous_until, addr + num_bytes);
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

void handle_destroy(sfl_t **ptr_list) {  
  sfl_t *list = *ptr_list;

  // free all elements of dll
  for (int i = 0; i < list->dlls->num_elements; ++i) {
    for (dll_node_t *curr = ((dll_t *)al_get(list->dlls, i))->head; curr;) {
      dll_node_t *tmp = curr;
      curr = curr->next;

      free(tmp);
    }
  }

  // free data contained in allocd blocks
  for (int i = 0; i < list->allocd_blocks->num_elements; ++i) {
    block_t *b = ((block_t*)al_get(list->allocd_blocks, i));
    free(b->data);
  }

  al_free(list->dlls);
  al_free(list->allocd_blocks);
  free(list);

  *ptr_list = NULL;
}

void handle_free(char *cmd, sfl_t *list) {
  if (!list) {
    fprintf(stderr, "Heap was not initialised. You are a massive idiot :)\n");
    return;
  }

  char sep[] = " ";
  char *p = strtok(cmd, sep);

  size_t addr = 0;

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

    ++arg_index;
    p = strtok(NULL, sep);
  }

  printf("Freeing address 0x%lx (%ld)\n", addr, addr);

  int block_idx = al_first_if(list->allocd_blocks, &addr, block_address_greater) - 1;

  if (block_idx < 0) {
    fprintf(stderr, "Invalid free.\n");
    return;
  }

  block_t *block = al_get(list->allocd_blocks, block_idx);

  if (block->start_addr != addr) {
    fprintf(stderr, "Invalid free.\n");
    return;
  }

  printf("Freeing from index %d\n", block_idx);

  size_t new_addr = block->start_addr;
  size_t new_size = block->block_size;

  printf("A bubuit.\n");

  al_erase(list->allocd_blocks, block_idx);

  printf("A bubuit v2.\n");

  insert_new_shard(list, new_addr, new_size);
}

void handle_write(char *cmd, sfl_t *list) {
  if (!list) {
    fprintf(stderr, "Heap was not initialised. You are a massive idiot :)\n");
    return;
  }

  char sep[] = " ";
  char *p = strtok(cmd, sep);

  size_t addr = 0;
  char *data;
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
      data = strdup(p);
    }
    else if (arg_index == 3) {
      size_t tmp_num_bytes = atoll(p);
      if (tmp_num_bytes) {
        num_bytes = tmp_num_bytes;
      }
    }

    ++arg_index;
    p = strtok(NULL, sep);
  }

  printf("Will write %lu bytes of '%s' into addr %lx\n", num_bytes, data, addr);

  int target_block_idx = al_last_if(list->allocd_blocks, &addr, block_address_less_equal);
  block_t *target_block = al_get(list->allocd_blocks, target_block_idx);
  int exact_match = target_block->start_addr <= addr && target_block->start_addr + target_block->block_size > addr;

  if (!exact_match) {
    fprintf(stderr, "Segmentation Fault (not alloc'd). Esti prost facut gramada\n");
    return;
  }

  size_t contiguous_until = target_block->start_addr + target_block->block_size;
  printf("111 Initially contig until %lu\n", contiguous_until);
  for (int i = target_block_idx + 1; i < list->allocd_blocks->num_elements; ++i) {
    block_t *next_block = al_get(list->allocd_blocks, i);
    if (next_block->start_addr == contiguous_until) {
      contiguous_until = next_block->start_addr + next_block->block_size;
      printf("contig until %lu\n", contiguous_until);
    }
    else {
      break;
    }
  }
  if (contiguous_until < addr + num_bytes) {
    fprintf(stderr, "Segmentation Fault (not all bytes alloc'd - contig. until %lu, needed %lu). Esti prost facut gramada\n", contiguous_until, addr + num_bytes);
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

    printf("Wrote char %c on index %d\n", data[i], index);
  }
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
    else if (starts_with(cmd, "PRINT")) {
      handle_print(list);
    }
    else if (starts_with(cmd, "MALLOC")) {
      handle_malloc(cmd, list);
    }
    else if (starts_with(cmd, "READ")) {
      handle_read(cmd, list);
    }
    else if (starts_with(cmd, "WRITE")) {
      handle_write(cmd, list);
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