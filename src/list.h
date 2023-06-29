#ifndef MY_LIST_HPP
#define MY_LIST_HPP

#include <stdlib.h>

typedef struct List {
    void* data;
    size_t size_of_element;
    size_t size;
    size_t capacity;
} List;

List* list_create(size_t element_size);

List* list_create_capacity(size_t element_size, size_t capacity);

void list_destroy(List* l);

void list_add(List* l, void* data);

void list_put(List* l, void* data, size_t pos);

void list_remove(List* l, size_t pos);

void* list_get(List* l, size_t pos);

void list_resize(List* l, size_t newSize);

void list_realloc(List* l, size_t newCapacity);

// convenience
void list_pop(List* l);
void list_push(List* l, void* data);
void list_dequeue(List* l);

#define list_foreach(item, l)                                                  \
    for (int keep = 1, count = 0; keep && count != (l)->size;                  \
         keep = 1, count++)                                                    \
        for (item = list_get((l), count); keep; keep = 0)

#endif
