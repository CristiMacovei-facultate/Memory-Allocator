#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "arraylist.h"
#include "structs.h"

//todo figure out how to do this shit without static buffers
// -20 points if not
#define CMD_LINE 256

int starts_with(char *cmd, char *string) {
  int n = strlen(string);
  for (int i = 0; i < n; ++i) {
    if (cmd[i] != string[i]) {
      return 0;
    }
  }

  return 1;
}

void handle_init(char *cmd, sfl_t **ptr_list) {
  char sep[] = " ";
  char *p = strtok(cmd, sep);

  uint64_t start_addr = 0;
  int num_lists = 8;
  uint64_t bytes_per_list = 1024;

  int i = 0; 
  while (i < 4 && p) {
    // printf("Arg %d: '%s'\n", i, p);
    
    // start address 
    if (i == 1) {
      uint64_t tmp_start_addr = atoi(p); // todo change this shit to uint64_t
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
      uint64_t tmp_bpl = atoi(p); // todo also change this to uitn64_t
      if (tmp_bpl) {
        bytes_per_list = tmp_bpl;
      }
    }

    ++i;
    p = strtok(NULL, sep);
  }

  sfl_t *list = malloc(sizeof(sfl_t));
  list->start_addr = start_addr;
  list->num_lists = num_lists;
  list->bytes_per_list = bytes_per_list;
  list->type = 0; //todo change this in the future
  
  list->dlls = al_create(num_lists, sizeof(dll_t));
  // printf("El size: %lu\n", list->dlls->element_size);
  uint64_t addr = list->start_addr;
  for (int i = 0; i < num_lists; ++i) {
    uint64_t block_size = ((uint64_t)8 << i);
    int num_blocks = bytes_per_list / block_size;
    // printf("Will alloc list of %lu blocks with %lu size each\n", num_blocks, block_size);

    dll_t *tmp = malloc(sizeof(dll_t));
    tmp->block_size = block_size;
    // printf("block size = %lu\n", block_size);
    tmp->head = NULL;
    tmp->tail = NULL;
    tmp->num_nodes = num_blocks;
    al_insert(list->dlls, i, tmp);
    free(tmp);
    // printf("set to %lu\n", ((dll_t *)al_get(list->dlls, i))->block_size);

    ((dll_t *)al_get(list->dlls, i))->head = NULL;
    dll_node_t *prev = NULL;
    for (int j = 0; j < num_blocks; ++j) {
      // printf("[d] Alloc block %d on addr %lu\n", i, addr);
      dll_node_t *new = malloc(sizeof(dll_node_t));
    
      new->next = NULL;
      new->prev = prev;
      new->start_addr = addr;
      addr += block_size;

      if (prev) {
        prev->next = new;
      }
      else {
        ((dll_t *)al_get(list->dlls, i))->head = new;
      }

      prev = new;
    }
    ((dll_t *)al_get(list->dlls, i))->tail = prev;
  }

  // printf("Finished alloc\n");

  *ptr_list = list;
}

void handle_print(char *cmd, sfl_t *list) {
  printf("SFL with %d lists of size %lu each\n", list->num_lists, list->bytes_per_list);

  for (int i = 0; i < list->num_lists; ++i) {
    printf("List %d with size %lu: [", i, ((dll_t *)al_get(list->dlls, i))->block_size);

    for (dll_node_t *node = ((dll_t *)al_get(list->dlls, i))->head; node; node = node->next) {
      printf("(addr = %lu), ", node->start_addr);
    }
    printf("]\n");
  }
}

void handle_malloc(char *cmd, sfl_t *list) {
  if (!list) {
    fprintf(stderr, "Heap was not initialised. You are a massive idiot :)\n");
    return;
  }

  char sep[] = " ";
  char *p = strtok(cmd, sep);

  uint64_t requested = 0;

  int i = 0; 
  while (i < 4 && p) {
    printf("Arg %d: '%s'\n", i, p);
    
    // start address 
    if (i == 1) {
      uint64_t tmp_requested = atoi(p); // todo change this shit to uint64_t
      if (tmp_requested) {
        requested = tmp_requested;
      }
    }

    ++i;
    p = strtok(NULL, sep);
  }

  if (!requested) {
    fprintf(stderr, "Cannot alloc 0 bytes\n");
    return;
  }

  printf("[d] Have to alloc %lu bytes\n", requested);
  for (int i = 0; i < list->dlls->num_elements; ++i) {
    dll_t *dll = al_get(list->dlls, i);

    if (dll->num_nodes == 0) {
      fprintf(stderr, "[WARN] Skipping list %d, zero blocks remaining\n", i);
      continue;
    }

    if (dll->block_size < requested) {
      continue;
    }

    // alloc head of dll
    dll_node_t *mallocd_node = dll->head;
    
    dll->head = dll->head->next;
    dll->num_nodes--;

    free(mallocd_node);

    printf("Will malloc on addr %lu\n", mallocd_node->start_addr);
    printf("Will break block of size %lu into %lu and %lu\n", dll->block_size, requested, dll->block_size - requested);
    printf("Remaining head on: %p\n", dll->head);

    // todo fragment block and move the shard to another dll (create if non-existent)
    // todo implement al_find_if
    // todo implement al_insert_first_if
    // todo implement structure to hold alloc'd blocks

    break;
  }

  printf(stderr, "Out of memory.\n");
}

void handle_free(char *cmd, sfl_t **ptr_list) {
  sfl_t *list = *ptr_list;
  for (int i = 0; i < list->num_lists; ++i) {
    for (dll_node_t *curr = ((dll_t *)al_get(list->dlls, i))->head; curr;) {
      dll_node_t *tmp = curr;
      curr = curr->next;

      free(tmp);
    }
  }

  al_free(list->dlls);
  free(list);

  *ptr_list = NULL;
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
      handle_print(cmd, list);
    }
    else if (starts_with(cmd, "MALLOC")) {
      handle_malloc(cmd, list);
    }
    else if (starts_with(cmd, "DESTROY_HEAP")) {
      handle_free(cmd, &list);
      return 0;
    }
  }

  return 0;
}