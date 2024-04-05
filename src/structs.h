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

typedef struct block_t {
	void *address;
	size_t size;
} block_t;

// Structure for a node in the segregated free list
typedef struct sfl_node_t {
	void *data;
	struct sfl_node_t *next, *prev;
} sfl_node_t;

// Structure for a segregated free list
typedef struct sfl_list_t {
	sfl_node_t *head;
	size_t size;
} sfl_list_t;

#endif /* STRUCTURES_H_ */
