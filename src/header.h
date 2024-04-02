#ifndef HEADER_H_
#define HEADER_H_

#include "structs.h"
#include "utils.h"

// Functions from src/func/heap.c

// Function to initialize the heap
//
// Parameters:
//   - heap_start: The starting address of the heap
//   -  lists_num: The number of segregated free lists
//   - bytes_per_list: The number of bytes per list
//   - heap_data: Pointer to store the allocated memory for the heap
//
// Returns:
//   - The array of segregated free lists
sfl_list_t *init_heap(size_t heap_start, size_t lists_num,
					  size_t bytes_per_list, void **heap_data);

// Function to free the memory of the heap
//
// Parameters:
//   - sfl_lists: The array of segregated free lists
//   -  lists_num: The number of segregated free lists
//   - heap_data: Pointer to the allocated memory for the heap
//   - allocated_blocks: Linked list of allocated blocks
void destroy_heap(sfl_list_t *sfl_lists, size_t lists_num, void *heap_data,
				  ll_list_t allocated_blocks);

// Functions from src/func/lists.c

// Function to add a node to the linked list of allocated blocks
//
// Parameters:
//   - sfl_lists: Pointer to the array of segregated free lists
//   - index: The index of the segregated free list
//   - block_size: The size of the block to add
//   - allocated_blocks: Pointer to the linked list of allocated blocks
//   - lists_num: Pointer to the number of segregated free lists
//
// Returns:
//   - The address of the block added
size_t add_ll_node(sfl_list_t **sfl_lists, size_t index, size_t block_size,
				   ll_list_t *allocated_blocks, size_t *lists_num);

// Function to add a node to the segregated free list
//
// Parameters:
//   - block_address: The address of the block to add
//   - block_size: The size of the block to add
//   - sfl_lists: Pointer to the array of segregated free lists
//   - lists_num: Pointer to the number of segregated free lists
void add_sfl_node(size_t block_address, size_t block_size,
				  sfl_list_t **sfl_lists, size_t *lists_num);

// Function to remove a node from the linked list of allocated blocks
//
// Parameters:
//   - allocated_blocks: Pointer to the linked list of allocated blocks
//   - block_address: The address of the block to remove
//   - heap_data: Pointer to the allocated memory for the heap
//   - start_address: The starting address of the heap
//
// Returns:
//   - A pointer to the node removed
ll_node_t *remove_ll_node(ll_list_t *allocated_blocks, size_t block_address,
						  void *heap_data, size_t start_address);

// Functions from src/func/memory.c

// Function to allocate memory using segregated free lists
//
// Parameters:
//   - block_size: The size of the memory to allocate
//   - sfl_lists: Pointer to the array of segregated free lists
//   -  lists_num: Pointer to the number of segregated free lists
//   - allocated_blocks: Pointer to the linked list of allocated blocks
//   - fragmentations: Pointer to the count of fragmentations
//   - malloc_calls: Pointer to the count of malloc calls
void malloc_f(sfl_list_t **sfl_lists, size_t *lists_num,
			  ll_list_t *allocated_blocks, size_t *fragmentations,
			  size_t *malloc_calls);

// Function to unite the block that needs to be freed to adjacent free blocks
//
// Parameters:
//   - lists_num: Pointer to the number of segregated free lists
//   - sfl_lists: Pointer to the array of segregated free lists
//	 - block_address: Pointer to the address of the block to unite
//	 - block_size: Pointer to the size of the block to unite
//   - heap_data: Pointer to the allocated memory for the heap
//   - start_address: The starting address of the heap
//
// Returns:
//	 - true if it united blocks, false otherwise
bool defragment(size_t *lists_num, sfl_list_t **sfl_lists,
				size_t *block_address, size_t *block_size, void *heap_data,
				size_t start_address);

// Function to free memory using segregated free lists
//
// Parameters:
//   - sfl_lists: Pointer to the array of segregated free lists
//   -  lists_num: Pointer to the number of segregated free lists
//   - allocated_blocks: Pointer to the linked list of allocated blocks
//   - free_calls: Pointer to the count of free calls
//   - reconstruct_type: The type of reconstruction to be done
//   - heap_data: Pointer to the allocated memory for the heap
//   - start_address: The starting address of the heap
void free_f(sfl_list_t **sfl_lists, size_t *lists_num,
			ll_list_t *allocated_blocks, size_t *free_calls,
			size_t reconstruct_type, void *heap_data, size_t start_address,
			size_t bytes_per_list);

// Functions from src/func/read-write.c

// Function to read from a block of memory and manage segmentation faults
//
// Parameters:
//   - allocated_blocks: The linked list of allocated blocks
//   - heap_data: Pointer to the allocated memory for the heap
//   - start_address: The starting address of the heap
//   - command: The command to be executed
//   - free_calls: The number of free calls
//   - fragmentations: The number of fragmentations
//   - malloc_calls: The number of malloc calls
//   - sfl_lists: The array of segregated free lists
//   - lists_num: The number of segregated free lists
//
// Returns:
//   - True if the command was executed successfully, false otherwise
bool read(ll_list_t allocated_blocks, void *heap_data, size_t start_address,
		  char *command, size_t free_calls, size_t fragmentations,
		  size_t malloc_calls, sfl_list_t *sfl_lists, size_t lists_num);

// Function to write to a block of memory and manage segmentation faults
//
// Parameters:
//   - allocated_blocks: The linked list of allocated blocks
//   - heap_data: Pointer to the allocated memory for the heap
//   - start_address: The starting address of the heap
//   - command: The command to be executed
//   - free_calls: The number of free calls
//   - fragmentations: The number of fragmentations
//   - malloc_calls: The number of malloc calls
//   - sfl_lists: The array of segregated free lists
//   - lists_num: The number of segregated free lists
//
// Returns:
//   - True if the command was executed successfully, false otherwise
bool write(ll_list_t allocated_blocks, void *heap_data, size_t start_address,
		   char *command, size_t free_calls, size_t fragmentations,
		   size_t malloc_calls, sfl_list_t *sfl_lists, size_t lists_num);

// Function to dump the memory statistics
//
// Parameters:
//   -  lists_num: The number of segregated free lists
//   - malloc_calls: The number of malloc calls
//   - fragmentations: The number of fragmentations
//   - free_calls: The number of free calls
//   - sfl_lists: The array of segregated free lists
//   - allocated_blocks: The linked list of allocated blocks
//   - start_address: The starting address of the heap
//   - heap_data: Pointer to the allocated memory for the heap
void dump_memory(size_t lists_num, size_t malloc_calls, size_t fragmentations,
				 size_t free_calls, sfl_list_t *sfl_lists,
				 ll_list_t allocated_blocks, size_t start_address,
				 void *heap_data);

// Functions from src/func/utils.c

// Function to find if two blocks come from the same parent block
//
// Parameters:
//   - first_address: The address of the first block
//   - second_address: The address of the second block
//   - heap_data: Pointer to the allocated memory for the heap
//   - bytes_per_list: The number of bytes per list
//
// Returns:
//   - True if the two blocks come from the same parent block, false otherwise
bool same_parent(size_t first_address, size_t second_address, void *heap_data,
				 size_t bytes_per_list);

// Function to read a block of memory placed in between quotation marks
//
// Returns:
//   - The block of memory read
char *read_text(void);

// Function to run the program
void run(void);

#endif /* HEADER_H_ */
