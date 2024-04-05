#ifndef HEADER_H_
#define HEADER_H_

#include "structs.h"
#include "utils.h"

// Functions from src/func/heap.c

// @brief Function to initialize the heap
// @param heap_start The starting address of the heap
// @param lists_num The number of segregated free lists
// @param bytes_per_list The number of bytes per list
// @param heap_data Pointer to store the allocated memory for the heap
// @return The array of segregated free lists
sfl_list_t *init_heap(size_t heap_start, size_t lists_num,
					  size_t bytes_per_list, void **heap_data);

// @brief Function to free the memory of the heap
// @param sfl_lists The array of segregated free lists
// @param  lists_num The number of segregated free lists
// @param heap_data Pointer to the allocated memory for the heap
// @param allocated_blocks Linked list of allocated blocks
void destroy_heap(sfl_list_t *sfl_lists, size_t lists_num, void *heap_data,
				  sfl_list_t allocated_blocks);

// Functions from src/func/lists.c

// @brief Function to add a node to the linked list of allocated blocks
// @param sfl_lists Pointer to the array of segregated free lists
// @param index The index of the segregated free list
// @param block_size The size of the block to add
// @param allocated_blocks Pointer to the linked list of allocated blocks
// @param lists_num Pointer to the number of segregated free lists
// @return The address of the block added
size_t add_ll_node(sfl_list_t **sfl_lists, size_t index, size_t block_size,
				   sfl_list_t *allocated_blocks, size_t *lists_num);

// @brief Function to add a node to the segregated free list
// @param block_address The address of the block to add
// @param block_size The size of the block to add
// @param sfl_lists Pointer to the array of segregated free lists
// @param lists_num Pointer to the number of segregated free lists
void add_sfl_node(size_t block_address, size_t block_size,
				  sfl_list_t **sfl_lists, size_t *lists_num);

// @brief Function to remove a node from the linked list of allocated blocks
// @param allocated_blocks Pointer to the linked list of allocated blocks
// @param block_address The address of the block to remove
// @param heap_data Pointer to the allocated memory for the heap
// @param start_address The starting address of the heap
// @return A pointer to the node removed
sfl_node_t *remove_ll_node(sfl_list_t *allocated_blocks, size_t block_address,
						   void *heap_data, size_t start_address);

// Functions from src/func/memory.c

// @brief Function to allocate memory using segregated free lists
// @param block_size The size of the memory to allocate
// @param sfl_lists Pointer to the array of segregated free lists
// @param  lists_num Pointer to the number of segregated free lists
// @param allocated_blocks Pointer to the linked list of allocated blocks
// @param fragmentations Pointer to the count of fragmentations
// @param malloc_calls Pointer to the count of malloc calls
void malloc_f(sfl_list_t **sfl_lists, size_t *lists_num,
			  sfl_list_t *allocated_blocks, size_t *fragmentations,
			  size_t *malloc_calls);

// @brief Function to unite the block that needs to be freed to adjacent free
// blocks
// @param lists_num Pointer to the number of segregated free lists
// @param sfl_lists Pointer to the array of segregated free lists
// @param block_address Pointer to the address of the block to unite
// @param block_size Pointer to the size of the block to unite
// @param heap_data Pointer to the allocated memory for the heap
// @param start_address The starting address of the heap
// @return True if it united blocks, false otherwise
bool defragment(size_t *lists_num, sfl_list_t **sfl_lists,
				size_t *block_address, size_t *block_size, void *heap_data,
				size_t start_address);

// @brief Function to free memory using segregated free lists
// @param sfl_lists Pointer to the array of segregated free lists
// @param  lists_num Pointer to the number of segregated free lists
// @param allocated_blocks Pointer to the linked list of allocated blocks
// @param free_calls Pointer to the count of free calls
// @param reconstruct_type The type of reconstruction to be done
// @param heap_data Pointer to the allocated memory for the heap
// @param start_address The starting address of the heap
void free_f(sfl_list_t **sfl_lists, size_t *lists_num,
			sfl_list_t *allocated_blocks, size_t *free_calls,
			size_t reconstruct_type, void *heap_data, size_t start_address,
			size_t bytes_per_list);

// Functions from src/func/read-write.c

// @brief Function to read from a block of memory and manage segmentation faults
// @param allocated_blocks The linked list of allocated blocks
// @param heap_data Pointer to the allocated memory for the heap
// @param start_address The starting address of the heap
// @param command The command to be executed
// @param free_calls The number of free calls
// @param fragmentations The number of fragmentations
// @param malloc_calls The number of malloc calls
// @param sfl_lists The array of segregated free lists
// @param lists_num The number of segregated free lists
// @return True if the command was executed successfully, false otherwise
bool read(sfl_list_t allocated_blocks, void *heap_data, size_t start_address,
		  char *command, size_t free_calls, size_t fragmentations,
		  size_t malloc_calls, sfl_list_t *sfl_lists, size_t lists_num);

// @brief Function to write to a block of memory and manage segmentation faults
// @param allocated_blocks The linked list of allocated blocks
// @param heap_data Pointer to the allocated memory for the heap
// @param start_address The starting address of the heap
// @param command The command to be executed
// @param free_calls The number of free calls
// @param fragmentations The number of fragmentations
// @param malloc_calls The number of malloc calls
// @param sfl_lists The array of segregated free lists
// @param lists_num The number of segregated free lists
// @return True if the command was executed successfully, false otherwise
bool write(sfl_list_t allocated_blocks, void *heap_data, size_t start_address,
		   char *command, size_t free_calls, size_t fragmentations,
		   size_t malloc_calls, sfl_list_t *sfl_lists, size_t lists_num);

// @brief Function to dump the memory statistics
// @param  lists_num The number of segregated free lists
// @param malloc_calls The number of malloc calls
// @param fragmentations The number of fragmentations
// @param free_calls The number of free calls
// @param sfl_lists The array of segregated free lists
// @param allocated_blocks The linked list of allocated blocks
// @param start_address The starting address of the heap
// @param heap_data Pointer to the allocated memory for the heap
void dump_memory(size_t lists_num, size_t malloc_calls, size_t fragmentations,
				 size_t free_calls, sfl_list_t *sfl_lists,
				 sfl_list_t allocated_blocks, size_t start_address,
				 void *heap_data);

// Functions from src/func/utils.c

// @brief Function to find if two blocks come from the same parent block
// @param first_address The address of the first block
// @param second_address The address of the second block
// @param heap_data Pointer to the allocated memory for the heap
// @param bytes_per_list The number of bytes per list
// @return True if the two blocks come from the same parent block, false
// otherwise
bool same_parent(size_t first_address, size_t second_address, void *heap_data,
				 size_t bytes_per_list);

// @brief Function to read a block of memory placed in between quotation marks
// @return The block of memory read
char *read_text(void);

// @brief Function to run the program
void run(void);

#endif /* HEADER_H_ */
