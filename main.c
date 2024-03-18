#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

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
  
  list->array = malloc(num_lists * sizeof(dll_node_t *));
  uint64_t addr = list->start_addr;
  for (int i = 0; i < num_lists; ++i) {
    uint64_t block_size = ((uint64_t)8 << i);
    uint64_t num_blocks = bytes_per_list / block_size; 

    list->array[i] = malloc(sizeof(dll_node_t));
    (list->array[i])->block_size = block_size;
    (list->array[i])->prev = NULL;
    (list->array[i])->next = NULL;
    (list->array[i])->start_addr = addr;
    addr += block_size;

    dll_node_t *prev = list->array[i];
    for (int i = 1; i < num_blocks; ++i) {
      dll_node_t *new = malloc(sizeof(dll_node_t));
    
      new->block_size = block_size;
      new->next = NULL;
      new->prev = prev;
      new->start_addr = addr;
      addr += block_size;

      prev->next = new;
      prev = new;
    }
  }

  *ptr_list = list;
}

void handle_print(char *cmd, sfl_t *list) {
  printf("SFL with %d lists of size %lu each\n", list->num_lists, list->bytes_per_list);

  for (int i = 0; i < list->num_lists; ++i) {
    printf("List %d: [", i);

    for (dll_node_t *node = list->array[i]; node; node = node->next) {
      printf("(sz = %lu, addr = %lu), ", node->block_size, node->start_addr);
    }
    printf("]\n");
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
      handle_print(cmd, list);
    }
  }

  return 0;
}