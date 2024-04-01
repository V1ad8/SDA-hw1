#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

// The size of the command from the input
#define COMMAND_SIZE 100

// The size of the text from the input
#define TEXT_SIZE 600

// Boolean type for the C language
typedef enum { false, true } bool;

// Structure for a node in the segregated free list
typedef struct sfl_node_t {
	void *data;
	size_t index; // The index of the node in the list
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
	size_t parent_id; // The id of the parent node
	size_t size; // The size of the data in the node
	struct ll_node_t *next, *prev;
} ll_node_t;

// Structure for a linked list
typedef struct ll_list_t {
	ll_node_t *head;
	size_t size;
} ll_list_t;

#endif /* STRUCTURES_H_ */
