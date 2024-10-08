// Nicolae-Cristian MACOVEI, Anul I, Grupa 312CAb

#ifndef ARRAYLIST_H_GUARD
#define ARRAYLIST_H_GUARD

#include <stdlib.h>
#include <stdint.h>

typedef struct arraylist {
	int num_elements;
	size_t capacity;
	size_t element_size;
	void *data;
} arraylist_t;

arraylist_t *al_create(int capacity, size_t size);

void *al_get(arraylist_t *list, int index);

void al_insert(arraylist_t *list, int index, void *src);

void al_append(arraylist_t *list, void *src);

void al_free(arraylist_t *list);

int al_first_if(arraylist_t *list, void *target,
				int cmp(const void *a, const void *b));

int al_last_if(arraylist_t *list, void *target,
			   int cmp(const void *a, const void *b));

void al_erase(arraylist_t *list, int index);

#endif
