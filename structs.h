#pragma once

#include <stdint.h>

typedef struct dll_node {
  struct dll_node *next;
  struct dll_node *prev;
  // uint64_t block_size;
  uint64_t start_addr;
} dll_node_t;

typedef struct doubly_linked_list {
  dll_node_t *head;
  dll_node_t *tail;

  int num_nodes;
  uint64_t block_size;
} dll_t;

typedef struct sfl {
  uint64_t start_addr;
  int num_lists;
  uint64_t bytes_per_list;
  uint8_t type;

  dll_t **dlls;
} sfl_t;