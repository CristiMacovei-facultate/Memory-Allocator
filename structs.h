#ifndef STRUCTS_H_GUARD
#define STRUCTS_H_GUARD

#include <stdint.h>

#include "arraylist.h"

typedef struct dll_node {
  struct dll_node *next;
  struct dll_node *prev;
  uint64_t start_addr;
} dll_node_t;

typedef struct doubly_linked_list {
  dll_node_t *head;
  dll_node_t *tail;

  int num_nodes;
  uint64_t block_size;
} dll_t;

typedef struct block {
  size_t block_size;
  size_t start_addr;
} block_t;

typedef struct sfl {
  uint64_t start_addr;
  uint8_t type;

  arraylist_t *dlls; // arraylist of dll_t
  arraylist_t *allocd_blocks; // array list of block_t 
} sfl_t;

#endif