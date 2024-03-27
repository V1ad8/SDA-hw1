#ifndef HEADER_H_
#define HEADER_H_

#include "structs.h"

// Function to initialize the heap
//
// Parameters:
//   - heap_start: The starting address of the heap
//   - number_of_sfl_lists: The number of segregated free lists
//   - bytes_per_list: The number of bytes per list
//   - heap_data: Pointer to store the allocated memory for the heap
//
// Returns:
//   - The array of segregated free lists
sfl_list_t *init_heap(size_t heap_start, size_t number_of_sfl_lists,
					  size_t bytes_per_list, void **heap_data);

// Function to allocate memory using segregated free lists
//
// Parameters:
//   - block_size: The size of the memory to allocate
//   - sfl_lists: Pointer to the array of segregated free lists
//   - number_of_sfl_lists: Pointer to the number of segregated free lists
//   - allocated_blocks: Pointer to the linked list of allocated blocks
//   - fragmentations: Pointer to the count of fragmentations
//   - malloc_calls: Pointer to the count of malloc calls
void malloc_f(size_t block_size, sfl_list_t **sfl_lists,
			  size_t *number_of_sfl_lists, ll_list_t *allocated_blocks,
			  size_t *fragmentations, size_t *malloc_calls);

// Function to free memory using segregated free lists
//
// Parameters:
//   - block_address: The address of the block to be freed
//   - allocated_blocks: Pointer to the linked list of allocated blocks
//   - sfl_lists: Pointer to the array of segregated free lists
//   - number_of_sfl_lists: Pointer to the number of segregated free lists
//   - heap_data: Pointer to the allocated memory for the heap
//   - start_address: The starting address of the heap
//   - free_calls: Pointer to the counter for the number of free calls
void simple_free(size_t block_address, ll_list_t *allocated_blocks,
				 sfl_list_t **sfl_lists, size_t *number_of_sfl_lists,
				 void *heap_data, size_t start_address, size_t *free_calls);

// Function to read a block of memory
//
// Parameters:
//   - allocated_blocks: Pointer to the linked list of allocated blocks
//   - block_address: The address of the block to be read
//   - read_size: The size of the block to be read
//   - heap_data: Pointer to the allocated memory for the heap
//   - start_address: The starting address of the heap
//
// Returns:
//   - true if the operation was successful, false otherwise
bool read(ll_list_t *allocated_blocks, size_t block_address, size_t read_size,
		  void *heap_data, size_t start_address);

// Function to write a block of memory
//
// Parameters:
//   - allocated_blocks: Pointer to the linked list of allocated blocks
//   - block_address: The address of the block to be written
//   - write_size: The size of the block to be written
//   - heap_data: Pointer to the allocated memory for the heap
//   - start_address: The starting address of the heap
//   - text: Pointer to the block of memory to be written
//
// Returns:
//   - true if the operation was successful, false otherwise
bool write(ll_list_t *allocated_blocks, size_t block_address, size_t write_size,
		   void *heap_data, size_t start_address, char *text);

// Function to dump the memory statistics
//
// Parameters:
//   - number_of_sfl_lists: The number of segregated free lists
//   - malloc_calls: The number of malloc calls
//   - fragmentations: The number of fragmentations
//   - free_calls: The number of free calls
//   - sfl_lists: The array of segregated free lists
//   - allocated_blocks: The linked list of allocated blocks
//   - start_address: The starting address of the heap
//   - heap_data: Pointer to the allocated memory for the heap
void dump_memory(size_t number_of_sfl_lists, size_t malloc_calls,
				 size_t fragmentations, size_t free_calls,
				 sfl_list_t *sfl_lists, ll_list_t allocated_blocks,
				 size_t start_address, void *heap_data);

// Function to free the memory of the heap
//
// Parameters:
//   - sfl_lists: The array of segregated free lists
//   - number_of_sfl_lists: The number of segregated free lists
//   - heap_data: Pointer to the allocated memory for the heap
//   - allocated_blocks: Linked list of allocated blocks
void destroy_heap(sfl_list_t *sfl_lists, size_t number_of_sfl_lists,
				  void *heap_data, ll_list_t allocated_blocks);

// Function to read a block of memory placed in between quotation marks
//
// Returns:
//   - The block of memory read
char *read_text(void);

// Function to run the program
void run();

#endif /* HEADER_H_ */