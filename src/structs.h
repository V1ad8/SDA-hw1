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

// Structure for a block in the heap
typedef struct block_t {
	void *address; // The address of the block
	size_t size; // The size of the block
} block_t;

// Structure for a node in the segregated free list
typedef struct node_t {
	void *data; // The data of the node
	struct node_t *next, *prev; // The next and previous nodes
} node_t;

// Structure for a segregated free list
typedef struct list_t {
	node_t *head; // The head of the list
	size_t size; // The size of the list
} list_t;

#endif /* STRUCTURES_H_ */
