#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include <stddef.h>

// Structure for a node in the segregated free list
typedef struct sfl_node_t {
	void *data;
	struct sfl_node_t *next, *prev;
} sfl_node_t;

// Structure for a segregated free list
typedef struct sfl_list_t {
	sfl_node_t *head;
	size_t size;
	size_t element_size; // The size of the elements in the list
} sfl_list_t;

// Structure for a node in the linked list
typedef struct ll_node_t {
	void *data;
	size_t size; // The number of elements stored in the current block
	struct ll_node_t *next, *prev;
} ll_node_t;

// Structure for a linked list
typedef struct ll_list_t {
	ll_node_t *head;
	size_t size;
} ll_list_t;

#endif /* STRUCTURES_H_ */