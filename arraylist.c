// Nicolae-Cristian MACOVEI, Anul I, Grupa 312CAb

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "arraylist.h"

arraylist_t *al_create(int capacity, size_t element_size)
{
	arraylist_t *new = (arraylist_t *)malloc(sizeof(arraylist_t));

	new->capacity = capacity;
	new->num_elements = 0;
	new->element_size = element_size;
	new->data = malloc(capacity * element_size);

	return new;
}

void al_resize(arraylist_t *list, size_t new_cap)
{
	list->capacity = new_cap;
	list->data = realloc(list->data, list->capacity * list->element_size);
}

void *al_get(arraylist_t *list, int index)
{
	return (void *)(((char *)list->data) + index * list->element_size);
}

void al_insert(arraylist_t *list, int index, void *src)
{
	list->num_elements++;

	if (list->num_elements > (int)list->capacity)
		al_resize(list, list->capacity * 2);

	if (index < list->num_elements) {
		for (int i = list->num_elements - 1; i > index; --i)
			memcpy(al_get(list, i), al_get(list, i - 1), list->element_size);
	}

	void *ptr = al_get(list, index);
	memcpy(ptr, src, list->element_size);

#ifdef DEBUG_MODE
	printf("[DEBUG] Now on %d dlls out of %lu\n",
		   list->num_elements, list->capacity);
#endif
}

void al_append(arraylist_t *list, void *src)
{
	al_insert(list, list->num_elements, src);
}

void al_free(arraylist_t *list)
{
	free(list->data);
	free(list);
}

// linear search, maybe could be changed to binary search
// if arraylist is kept in order
int al_first_if(arraylist_t *list, void *target,
				int cmp(const void*, const void*))
{
	for (int i = 0; i < list->num_elements; ++i) {
		void *curr = al_get(list, i);
		if (cmp(target, curr) == 1)
			return i;
	}

	return list->num_elements;
}

int al_last_if(arraylist_t *list, void *target,
			   int cmp(const void*, const void*))
{
	int ans = 0;
	for (int i = 0; i < list->num_elements; ++i) {
		void *curr = al_get(list, i);
		if (cmp(target, curr) == 1)
			ans = i;
		else
			break;
	}

	return ans;
}

void al_erase(arraylist_t *list, int index)
{
	if (list->num_elements == 0)
		return;

	list->num_elements--;
	if (index < list->num_elements) {
		for (int i = index; i < list->num_elements; ++i) {
			void *first = al_get(list, i);
			void *next = al_get(list, i + 1);

			memcpy(first, next, list->element_size);
		}
	}

	if (list->num_elements > 0 &&
		(size_t)list->num_elements * 2 < list->capacity)
		al_resize(list, list->capacity / 2);
}
