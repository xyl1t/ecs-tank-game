#include "list.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

List* list_create_capacity(size_t size_of_element, size_t capacity) {
    List* l = (List*)malloc(sizeof(List));

    l->size_of_element = size_of_element;
    l->size = 0;
    l->capacity = capacity;
    l->data = malloc(size_of_element * l->capacity);
    return l;
}

List* list_create(size_t size_of_element) {
    return list_create_capacity(size_of_element, 2);
}

void list_destroy(List* l) {
    l->size = 0;
    l->capacity = 0;
    l->size_of_element = 0;
    free(l->data);
    free(l);
}

void list_add(List* l, void* data) {
    if (l->size >= l->capacity) {
        l->capacity *= 2;
        l->data = realloc(l->data, l->size_of_element * l->capacity);
    }

    // uint8_t* ptr = &((uint8_t*)l->data)[(l->size) * l->size_of_element];
    uint8_t* ptr = (uint8_t*)l->data + (l->size * l->size_of_element);
    memcpy(ptr, data, l->size_of_element);
    l->size++;
}

void list_put(List* l, void* data, size_t pos) {
    assert(pos >= 0 && "pos is less than 0");

    if (pos >= l->size) {
        l->size = pos + 1;
    }
    if (pos >= l->capacity) {
        l->capacity = pos + 1;
        l->data = realloc(l->data, l->size_of_element * l->capacity);
    }

    uint8_t* ptr = (uint8_t*)l->data + (pos * l->size_of_element);
    memcpy(ptr, data, l->size_of_element);
}

void list_remove(List* l, size_t pos) {
    assert(pos < l->size && pos >= 0 && "pos is outside of range");

    memmove((uint8_t*)l->data + pos * l->size_of_element,
        (uint8_t*)l->data + (pos + 1) * l->size_of_element,
        (l->size - pos) * l->size_of_element);
    l->size--;
}

void* list_get(List* l, size_t pos) {
    return (uint8_t*)l->data + (pos * l->size_of_element);
}

void list_resize(List* l, size_t newSize) {
    l->size = newSize;
    l->capacity = newSize;
    l->data = realloc(l->data, l->size_of_element * newSize);
}

void list_realloc(List* l, size_t newCapacity) {
    l->capacity = newCapacity;
    l->data = realloc(l->data, l->size_of_element * newCapacity);
}

void list_pop(List* l) { l->size--; }

void list_push(List* l, void* data) { list_add(l, data); }

void list_dequeue(List* l) { list_remove(l, 0); }
