#pragma once
#include <stdint.h>
#include <stdlib.h>

#define INITIAL_CAPACITY 32

typedef struct {
	void** values;
	uint32_t count;
	uint32_t capacity;
} List;

typedef enum {
	LIST_OK,
	LIST_ALLOC_FAILED,
	LIST_NULL_REFRENCE,
	LIST_NOT_INITIALIZED,
	LIST_OUT_OF_RANGE,
	LIST_DISPOSE_FAILED
} ListResult;

ListResult list_init(List* list);
ListResult list_add(List* list, void* value);
ListResult list_remove(List* list, uint32_t index);
ListResult list_remove_all(List* list, uint32_t index);
ListResult list_resize(List* list, uint32_t capacity);

ListResult list_dispose_child(List* list, uint32_t index);
ListResult list_dispose_children(List* list);
ListResult list_dispose(List* list);

const char* list_result_message(ListResult result);