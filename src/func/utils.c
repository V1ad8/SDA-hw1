#include "../header.h"

bool is_power(size_t x)
{
	for (size_t i = 8; i <= x; i = i << 1)
		if (i == x)
			return true;

	return false;
}

char *read_text(void)
{
	// Allocate memory for the block
	char *text = malloc(TEXT_SIZE * sizeof(char));
	DIE(!text, "Malloc failed while allocating text");

	// Initialize an index for the block
	size_t i = 0;

	// Read the characters from the input, one by one
	char c;

	// Skip characters until the opening quotation mark is found
	do {
		c = fgetc(stdin);
	} while (c != '"');

	// Read characters until the closing quotation mark is found
	do {
		c = fgetc(stdin);
		text[i++] = c;
	} while (c != '"');

	// Add the null terminator
	text[i - 1] = '\0';

	return text;
}

void run(void)
{
	// Initialize the lists and the heap data
	sfl_list_t *sfl_lists = NULL;
	void *heap_data = NULL;
	ll_list_t allocated_blocks = { NULL, 0 };

	// Initialize the memory statistics
	size_t malloc_calls = 0;
	size_t free_calls = 0;
	size_t fragmentations = 0;

	// Declare the variables read for the input
	size_t start_address, lists_num, bytes_per_list, reconstruct_type;

	// Allocate memory for the command
	char *command = (char *)malloc(COMMAND_SIZE * sizeof(char));

	while (true) {
		// Read the command from the input
		scanf("%s", command);

		if (!strcmp(command, "INIT_HEAP")) {
			// Read the parameters for the INIT_HEAP command
			scanf("%lx %lu %lu %lu", &start_address, &lists_num,
			      &bytes_per_list, &reconstruct_type);

			// Initialize the heap
			sfl_lists = init_heap(start_address, lists_num,
								  bytes_per_list, &heap_data);
		} else if (!strcmp(command, "DUMP_MEMORY")) {
			// Dump the memory statistics
			dump_memory(lists_num, malloc_calls, fragmentations,
						free_calls, sfl_lists, allocated_blocks,
						start_address, heap_data);
		} else if (!strcmp(command, "MALLOC")) {
			// Allocate memory
			malloc_f(&sfl_lists, &lists_num, &allocated_blocks,
					 &fragmentations, &malloc_calls);
		} else if (!strcmp(command, "FREE")) {
			// Free memory
			free_f(&sfl_lists, &lists_num, &allocated_blocks,
			       &free_calls, reconstruct_type, heap_data,
			       start_address);
		} else if (!strcmp(command, "READ")) {
			// Read the block
			if (!read(allocated_blocks, heap_data, start_address,
					  command, free_calls, fragmentations,
					  malloc_calls, sfl_lists, lists_num))
				return;
		} else if (!strcmp(command, "WRITE")) {
			// Write the block
			if (!write(allocated_blocks, heap_data, start_address,
					   command, free_calls, fragmentations,
					   malloc_calls, sfl_lists, lists_num))
				return;
		} else if (!strcmp(command, "DESTROY_HEAP")) {
			// Destroy the heap
			destroy_heap(sfl_lists, lists_num, heap_data,
						 allocated_blocks);

			// Exit the program
			free(command);
			return;
		}
	}
}
