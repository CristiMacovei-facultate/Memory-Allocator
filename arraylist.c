#include <string.h>
#include <stdlib.h>

typedef struct arraylist {
  int capacity;
  int num_elements;
  size_t element_size;
  void *data;
} arraylist_t;

arraylist_t* al_create(int capacity, size_t element_size) {
  arraylist_t *new = (arraylist_t*)malloc(sizeof(arraylist_t));  
  
  new->capacity = capacity;
  new->num_elements = 0;
  new->element_size = element_size;
  new->data = malloc(capacity * element_size);

  return new;
}

void *al_get(arraylist_t *list, int index) {
  return (void*)(((char*)list->data) + index * list->element_size);
}

void al_set(arraylist_t *list, int index, void *src) {
  void *ptr = al_get(list, index);
  memcpy(ptr, src, list->element_size);
}

void al_free(arraylist_t *list) {
  free(list->data);
  free(list);
}