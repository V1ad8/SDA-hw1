#ifndef HEADER_H_
#define HEADER_H_

#include <stddef.h>
#include "structs.h"

// Function to initialize the heap
//
// Parameters:
//	 - heap_start: The starting address of the heap
//	 - num_lists: The number of segregated free lists
//	 - bytes_per_list: The number of bytes per list
//	 - heap: Pointer to store the allocated memory for the heap
//
// Returns:
//	 - The array of segregated free lists
sfl_list_t *init_heap(size_t heap_start, size_t num_lists,
		      size_t bytes_per_list, void **heap);

// Function to allocate memory using segregated free lists
//
// Parameters:
//   - size: The size of the memory to allocate
//   - lists: Pointer to the array of segregated free lists
//   - num_lists: Pointer to the number of segregated free lists
//   - allocated_blocks: Pointer to the linked list of allocated blocks
//   - fragmentations: Pointer to the count of fragmentations
//   - malloc_calls: Pointer to the count of malloc calls
void malloc_f(size_t size, sfl_list_t **lists, size_t *num_lists,
	      ll_list_t *allocated_blocks, size_t *fragmentations,
	      size_t *malloc_calls);

// Function to free memory using segregated free lists
//
// Parameters:
//   - address: The address of the block to be freed
//   - allocated_blocks: Pointer to the linked list of allocated blocks
//   - lists: Pointer to the array of segregated free lists
//   - num_lists: Pointer to the number of segregated free lists
//   - heap: Pointer to the allocated memory for the heap
//   - start_address: The starting address of the heap
//   - free_calls: Pointer to the counter for the number of free calls
void simple_free(size_t address, ll_list_t *allocated_blocks,
		 sfl_list_t **lists, size_t *num_lists, void *heap,
		 size_t start_address, size_t *free_calls);

// Function to read a block of memory
//
// Parameters:
//   - allocated_blocks: Pointer to the linked list of allocated blocks
//   - address: The address of the block to be read
//   - size: The size of the block to be read
//   - heap: Pointer to the allocated memory for the heap
//   - start_address: The starting address of the heap
void read(ll_list_t *allocated_blocks, size_t address, size_t size, void *heap,
	  size_t start_address);

// Function to write a block of memory
//
// Parameters:
//   - allocated_blocks: Pointer to the linked list of allocated blocks
//   - address: The address of the block to be written
//   - size: The size of the block to be written
//   - heap: Pointer to the allocated memory for the heap
//   - start_address: The starting address of the heap
//   - block: Pointer to the block of memory to be written
//
// Returns:
//   - true if the write was successful, false otherwise
bool write(ll_list_t *allocated_blocks, size_t address, size_t size, void *heap,
	   size_t start_address, char *block);

// Function to dump the memory statistics
//
// Parameters:
//   - num_lists: The number of segregated free lists
//   - malloc_calls: The number of malloc calls
//   - fragmentations: The number of fragmentations
//   - free_calls: The number of free calls
//   - lists: The array of segregated free lists
//   - allocated_blocks: The linked list of allocated blocks
//   - start_address: The starting address of the heap
//   - heap: Pointer to the allocated memory for the heap
void dump_memory(size_t num_lists, size_t malloc_calls, size_t fragmentations,
		 size_t free_calls, sfl_list_t *lists,
		 ll_list_t allocated_blocks, size_t start_address, void *heap);

// Function to free the memory of the heap
//
// Parameters:
//   - lists: The array of segregated free lists
//   - num_lists: The number of segregated free lists
//   - heap: Pointer to the allocated memory for the heap
//   - allocated_blocks: Linked list of allocated blocks
void destroy_heap(sfl_list_t *lists, size_t num_lists, void *heap,
		  ll_list_t allocated_blocks);

// Function to remove the quotation marks from a string
//
// Parameters:
//   - string: The string to remove the quotation marks from
void remove_quotation_marks(char *string);

// Function to read a block of memory placed in between quotation marks
//
// Returns:
//   - The block of memory read
char *read_block(void);

#endif /* HEADER_H_ */