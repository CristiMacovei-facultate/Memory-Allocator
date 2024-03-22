/* ----------------------------------------------
implementation of a circular doubly linked list
---------------------------------------------- */

#include <stdlib.h>

#ifdef DEBUG_MODE
#include <stdio.h>
#endif 

#include "dll.h"

dll_t *dll_create_empty(size_t block_size) {
  dll_t *new_dll = (dll_t *)malloc(sizeof(dll_t));
  new_dll->block_size = block_size;
  new_dll->num_nodes = 0;

  new_dll->head = NULL;

  return new_dll;
}

dll_t *dll_create_from_node (size_t block_size, dll_node_t *new_node) {
  dll_t *new_dll = dll_create_empty(block_size);
  
  new_dll->num_nodes = 1;
  new_node->next = new_node;
  new_node->prev = new_node;
  new_dll->head = new_node;

  return new_dll;
}

void dll_insert_first(dll_t *list, dll_node_t *new_node) {
  int num_nodes = list->num_nodes;
  ++(list->num_nodes);

  if (num_nodes == 0) {
    new_node->next = new_node->prev = list->head = new_node;
    return;
  }

  new_node->next = list->head;
  new_node->prev = list->head->prev;

  list->head->prev->next = new_node;
  list->head->prev = new_node;

  list->head = new_node;
}

void dll_insert_last(dll_t *list, dll_node_t *new_node) {
  int num_nodes = list->num_nodes;
  ++(list->num_nodes);

  if (num_nodes == 0) {
    new_node->next = new_node->prev = list->head = new_node;
    return;
  }

  dll_node_t *tail = list->head->prev;

  new_node->prev = tail;
  new_node->next = tail->next;
  tail->next->prev = new_node;
  tail->next = new_node;
}

void dll_insert_before_first_if(dll_t *list, dll_node_t *new_node) {
  int num_nodes = list->num_nodes;
  ++(list->num_nodes);

  if (num_nodes == 0) {
    new_node->next = new_node->prev = list->head = new_node;
    return;
  }

  dll_node_t *head = list->head;
  dll_node_t *node = head;
  do {
    if (node->start_addr > new_node->start_addr) {
      break;
    }

    node = node->next;
  } while (node != head);

#ifdef DEBUG_MODE
  printf("[DEBUG] Will insert before node 0x%lx\n", node->start_addr);
#endif

  new_node->next = node;
  new_node->prev = node->prev;
  node->prev->next = new_node;
  node->prev = new_node;

  // second condition needed because case when no element has a higher address than target
  // and it would insert before head
  if (node == list->head && node->start_addr > new_node->start_addr) {
    list->head = new_node;
  }
}

dll_node_t *dll_pop_first(dll_t *list) {
  if (list->num_nodes == 0) {
    return NULL;
  }

  --(list->num_nodes);

  dll_node_t *old_head = list->head;
  dll_node_t *tmp_next = old_head->next;
  dll_node_t *tmp_prev = old_head->prev;

  tmp_prev->next = old_head->next;
  tmp_next->prev = old_head->prev;

  list->head = list->num_nodes > 0 ? old_head->next : NULL;

  return old_head;
}
