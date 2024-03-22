#ifndef STRUCTS_H_GUARD
#define STRUCTS_H_GUARD

#include <stdint.h>

#include "dll.h"
#include "arraylist.h"


typedef struct block {
  size_t block_size;
  size_t start_addr;

  void *data;
} block_t;

typedef struct sfl {
  uint64_t start_addr;
  uint8_t type;

  arraylist_t *dlls; // arraylist of dll_t
  arraylist_t *allocd_blocks; // array list of block_t 

  // for memdump
  size_t total_mem;
  size_t total_allocd;
  int num_allocs;
  int num_frees;
  int num_fragmentations;
} sfl_t;

#endif