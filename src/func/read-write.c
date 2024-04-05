#include "../header.h"

bool read(sfl_list_t allocated_blocks, void *heap_data, size_t start_address,
		  char *command, size_t free_calls, size_t fragmentations,
		  size_t malloc_calls, sfl_list_t *sfl_lists, size_t lists_num)
{
	// Declare the variables read for the input
	size_t block_address, read_size;

	// Read the address and the size of the block to be read
	scanf("%lx %lu", &block_address, &read_size);

	// Allocate memory for the block
	char *text = malloc(read_size + 1);
	DIE(!text, "Malloc failed while allocating text");

	// Make an index for the text
	size_t j = 0;

	// Counter for the number of bytes read
	size_t read = 0;

	// Make an index for the allocated block
	size_t i = 0;

	// Find the block with the given address
	for (sfl_node_t *current = allocated_blocks.head; current;
		 current = current->next) {
		// Check if the address is found
		if (block_address !=
			(size_t)((block_t *)(size_t)current->data)->address -
				(size_t)heap_data + start_address)
			continue;

		// Copy the data from the allocated memory to the text
		for (i = 0, read = 0;
			 i < read_size && i < ((block_t *)(size_t)current->data)->size;
			 i++, read++)
			text[j++] =
				*((char *)((block_t *)(size_t)current->data)->address + i);

		// Update the size and address
		read_size -= read;
		block_address += read;

		// Check if the text is read completely
		if (read_size == 0) {
			// Add the null terminator
			text[j] = '\0';

			// Print the text
			printf("%s\n", text);

			// Free the memory of the text
			free(text);

			// Return true if the text is read completely
			return true;
		}
	}

	// If the address is not found, print an error message
	printf("Segmentation fault (core dumped)\n");

	// Free the memory of the text
	free(text);

	// Dump the memory statistics
	dump_memory(lists_num, malloc_calls, fragmentations, free_calls, sfl_lists,
				allocated_blocks, start_address, heap_data);

	// Destroy the heap
	destroy_heap(sfl_lists, lists_num, heap_data, allocated_blocks);

	// Free the command
	free(command);

	// Return false if the text is not read completely
	return false;
}

bool write(sfl_list_t allocated_blocks, void *heap_data, size_t start_address,
		   char *command, size_t free_calls, size_t fragmentations,
		   size_t malloc_calls, sfl_list_t *sfl_lists, size_t lists_num)
{
	// Declare the variables read for the input
	size_t block_address, write_size;

	// Read the address and the size of the block to be written
	scanf("%lx", &block_address);

	// Read the block
	char *text = read_text();

	// Read the size of the block to be written
	scanf("%lu", &write_size);

	// Calculate the original text size
	size_t text_size = strlen(text);

	// Make an index for the allocated block
	size_t i;

	// Make an index for the soon-to-be-written block
	size_t j = 0;

	// Find the block with the given address
	for (sfl_node_t *current = allocated_blocks.head; current;
		 current = current->next) {
		// Check if the address is found
		if (block_address !=
			(size_t)((block_t *)(size_t)current->data)->address -
				(size_t)heap_data + start_address)
			continue;

		// Copy the data from the text to the allocated memory
		for (i = 0;
			 i < write_size && i < ((block_t *)(size_t)current->data)->size &&
			 i < text_size;
			 i++, j++) {
			*((char *)((block_t *)(size_t)current->data)->address + i) =
				text[j];
		}

		// Update the size, address and text size
		write_size -= i;
		text_size -= i;
		block_address += i;

		// Check if the text is written completely and return true
		if (write_size == 0 || text_size == 0) {
			// Free the memory of the block
			free(text);

			// Return true if the text is written completely
			return true;
		}
	}

	// If the address is not found, print an error message
	printf("Segmentation fault (core dumped)\n");

	// Dump the memory statistics
	dump_memory(lists_num, malloc_calls, fragmentations, free_calls, sfl_lists,
				allocated_blocks, start_address, heap_data);

	// Destroy the heap
	destroy_heap(sfl_lists, lists_num, heap_data, allocated_blocks);

	// Free the command and the block
	free(command);
	free(text);

	// Return false if the block is not written completely
	return false;
}

void dump_memory(size_t lists_num, size_t malloc_calls, size_t fragmentations,
				 size_t free_calls, sfl_list_t *sfl_lists,
				 sfl_list_t allocated_blocks, size_t start_address,
				 void *heap_data)
{
	printf("+++++DUMP+++++\n");

	// Calculate the number of free blocks and the total free memory
	size_t free_blocks = 0;
	size_t free_memory = 0;
	for (size_t i = 0; i < lists_num; i++) {
		free_blocks += sfl_lists[i].size;
		free_memory +=
			sfl_lists[i].size * ((block_t *)sfl_lists[i].head->data)->size;
	}

	// Calculate the total allocated memory
	size_t allocated_memory = 0;
	for (sfl_node_t *current = allocated_blocks.head; current;
		 current = current->next) {
		allocated_memory += ((block_t *)current->data)->size;
	}

	// Print the total memory, total allocated memory, total free memory,
	// number of free blocks, number of allocated blocks, number of malloc
	// calls, number of fragmentations, and number of free calls

	printf("Total memory: %lu bytes\n", free_memory + allocated_memory);
	printf("Total allocated memory: %lu bytes\n", allocated_memory);
	printf("Total free memory: %lu bytes\n", free_memory);
	printf("Free blocks: %lu\n", free_blocks);
	printf("Number of allocated blocks: %lu\n", malloc_calls - free_calls);
	printf("Number of malloc calls: %lu\n", malloc_calls);
	printf("Number of fragmentations: %lu\n", fragmentations);
	printf("Number of free calls: %lu\n", free_calls);

	// Print blocks with their respective sizes and number of free blocks
	for (size_t i = 0; i < lists_num; i++) {
		printf("Blocks with %lu bytes - %lu free block(s) : ",
			   ((block_t *)sfl_lists[i].head->data)->size, sfl_lists[i].size);

		// Print the addresses of the free blocks
		for (sfl_node_t *current = sfl_lists[i].head; current;
			 current = current->next) {
			printf("0x%lx", (size_t)((block_t *)current->data)->address -
								(size_t)heap_data + start_address);

			// Print a space if there are more blocks
			if (current->next)
				printf(" ");
		}

		printf("\n");
	}

	// Print the addresses of the allocated blocks
	printf("Allocated blocks :");
	if (allocated_blocks.head) {
		printf(" ");

		for (sfl_node_t *current = allocated_blocks.head; current;
			 current = current->next) {
			printf("(0x%lx - %lu)",
				   (size_t)((block_t *)current->data)->address -
					   (size_t)heap_data + start_address,
				   ((block_t *)current->data)->size);

			// Print a space if there are more blocks
			if (current->next)
				printf(" ");
		}
	}

	printf("\n-----DUMP-----\n");
}
