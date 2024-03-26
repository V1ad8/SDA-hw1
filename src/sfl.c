#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "header.h"

int main(void)
{
	// Initialize the lists and the data
	sfl_list_t *list = NULL;
	void *data = NULL;
	ll_list_t allocated_blocks = { NULL, 0 };

	// Initialize the memory statistics
	size_t malloc_calls = 0;
	size_t free_calls = 0;
	size_t fragmentations = 0;

	// Initialize the variables for the input
	size_t start_address, num_lists, bytes_per_list, reconstruct, size,
		address;

	// Allocate memory for the command
	char *command = (char *)malloc(COMMAND_SIZE * sizeof(char));

	while (1) {
		// Read the command from the input
		scanf("%s", command);

		if (!strcmp(command, "INIT_HEAP")) {
			// Read the parameters for the INIT_HEAP command
			scanf("%lx %lu %lu %lu", &start_address, &num_lists,
			      &bytes_per_list, &reconstruct);

			// Initialize the heap
			list = init_heap(start_address, num_lists,
					 bytes_per_list, &data);
		} else if (!strcmp(command, "DUMP_MEMORY")) {
			// Dump the memory statistics
			dump_memory(num_lists, malloc_calls, fragmentations,
				    free_calls, list, allocated_blocks,
				    start_address, data);
		} else if (!strcmp(command, "MALLOC")) {
			// Read the size of the block to be allocated
			scanf("%lu", &size);

			malloc_f(size, &list, &num_lists, &allocated_blocks,
				 &fragmentations, &malloc_calls);
		} else if (!strcmp(command, "FREE")) {
			// Read the address of the block to be freed
			scanf("%lx", &address);

			// Free the memory of the block
			if (reconstruct) {
			} else {
				simple_free(address, &allocated_blocks, &list,
					    &num_lists, data, start_address,
					    &free_calls);
			}
		} else if (!strcmp(command, "READ")) {
			// Read the address and the size of the block to be read
			scanf("%lx %lu", &address, &size);

			// Read the block
			if (!read(&allocated_blocks, address, size, data,
				  start_address)) {
				dump_memory(num_lists, malloc_calls,
					    fragmentations, free_calls, list,
					    allocated_blocks, start_address,
					    data);
			}

		} else if (!strcmp(command, "WRITE")) {
			// Read the address and the size of the block to be written
			scanf("%lx", &address);

			// Read the block
			char *block = read_block();

			// Read the size of the block to be written
			scanf("%lu", &size);

			// Write the block
			if (!write(&allocated_blocks, address, size, data,
				   start_address, block)) {
				dump_memory(num_lists, malloc_calls,
					    fragmentations, free_calls, list,
					    allocated_blocks, start_address,
					    data);
			}
			free(block);
		} else if (!strcmp(command, "DESTROY_HEAP")) {
			// Destroy the heap
			destroy_heap(list, num_lists, data, allocated_blocks);

			// Exit the program
			free(command);
			return 0;
		}
	}
}