#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "utils.h"

sfl_list_t *init_heap(size_t heap_start, size_t lists_num,
		      size_t bytes_per_list, void **heap_data)
{
	// Allocate memory for the heap
	*heap_data = malloc(lists_num * bytes_per_list);
	DIE(!*heap_data, "Malloc failed while allocating heap_data");

	// Set the starting address of the heap
	heap_start = (size_t)*heap_data;

	// Allocate memory for the segregated free lists
	sfl_list_t *sfl_lists = malloc(sizeof(sfl_list_t) * lists_num);
	DIE(!sfl_lists, "Malloc failed while allocating sfl_lists");

	// Initialize each segregated free list
	for (size_t i = 0; i < lists_num; i++) {
		// Calculate the element size and size of the current list
		sfl_lists[i].element_size = 8 * (1 << i);

		sfl_lists[i].size = bytes_per_list / sfl_lists[i].element_size;

		// Create the head node for the current list
		sfl_node_t *previous = malloc(sizeof(sfl_node_t));
		DIE(!previous,
		    "Malloc failed while allocating node for sfl_lists[i]");
		sfl_lists[i].head = previous;
		previous->data = (void *)(heap_start + i * bytes_per_list);
		previous->prev = NULL;

		// Create the remaining nodes for the current list
		for (size_t j = 1; j < sfl_lists[i].size; j++) {
			sfl_node_t *current = malloc(sizeof(sfl_node_t));
			DIE(!current,
			    "Malloc failed while allocating node for sfl_lists[i]");

			// Set the data of the current node
			current->data =
				(void *)(heap_start + i * bytes_per_list +
					 j * sfl_lists[i].element_size);

			// Connect the current node to the previous one
			previous->next = current;
			current->prev = previous;

			// Move
			previous = current;
		}

		// Set the next pointer of the last node to NULL
		previous->next = NULL;
	}

	return sfl_lists;
}

size_t add_ll_node(sfl_list_t **sfl_lists, size_t index, size_t block_size,
		   ll_list_t *allocated_blocks, size_t *lists_num)
{
	// Allocate memory for a new node in the allocated blocks list
	ll_node_t *new_ll = malloc(sizeof(ll_node_t));
	DIE(!new_ll, "Malloc failed while allocating node");

	// Initialise the data of the current node
	new_ll->data = (*sfl_lists)[index].head->data;
	new_ll->size = block_size;
	new_ll->next = NULL;
	new_ll->prev = NULL;

	ll_node_t *last_ll = allocated_blocks->head;
	if (!last_ll) {
		allocated_blocks->head = new_ll;
	} else if (new_ll->data < last_ll->data) {
		last_ll->prev = new_ll;
		new_ll->next = last_ll;

		allocated_blocks->head = new_ll;
	} else {
		// Move to the appropriate position
		while (last_ll->next && last_ll->next->data < new_ll->data)
			last_ll = last_ll->next;

		// Add the current node to the allocated blocks list
		new_ll->next = last_ll->next;
		new_ll->prev = last_ll;

		if (last_ll->next)
			last_ll->next->prev = new_ll;

		last_ll->next = new_ll;
	}

	// Update the number of allocated blocks
	allocated_blocks->size += 1;

	// Remove the first node from the segregated free list
	sfl_node_t *first_sfl = (*sfl_lists)[index].head;
	(*sfl_lists)[index].head = (*sfl_lists)[index].head->next;
	if ((*sfl_lists)[index].head)
		(*sfl_lists)[index].head->prev = NULL;

	// Update the number of free blocks in the list
	(*sfl_lists)[index].size -= 1;

	// Check if the list is empty
	if ((*sfl_lists)[index].size == 0) {
		// Update the number of lists
		*lists_num -= 1;

		// Move the lists to the left
		for (size_t j = index; j < *lists_num; j++)
			(*sfl_lists)[j] = (*sfl_lists)[j + 1];

		// Reallocate memory for the segregated free lists
		*sfl_lists =
			realloc(*sfl_lists, *lists_num * sizeof(sfl_list_t));
		DIE(!sfl_lists, "Realloc failed while reallocating sfl_lists");
	}

	// Free the memory of the removed node
	free(first_sfl);

	return (size_t)new_ll->data;
}

void add_sfl_node(size_t block_address, size_t block_size,
		  sfl_list_t **sfl_lists, size_t *lists_num)
{
	// Allocate memory for a new node in the segregated free list
	sfl_node_t *new_sfl = malloc(sizeof(sfl_node_t));
	DIE(!new_sfl, "Malloc failed while allocating node");

	// Set the data of the current node
	new_sfl->data = (void *)(block_address);

	// Find the list which matches the remaining size
	for (size_t j = 0; j < *lists_num; j++) {
		if ((*sfl_lists)[j].element_size == block_size) {
			sfl_node_t *last_sfl = (*sfl_lists)[j].head;

			if (new_sfl->data < last_sfl->data) {
				new_sfl->next = (*sfl_lists)[j].head;
				new_sfl->prev = NULL;
				(*sfl_lists)[j].head->prev = new_sfl;
				(*sfl_lists)[j].head = new_sfl;
			} else {
				// Move to the appropriate position
				while (last_sfl->next &&
				       last_sfl->next->data < new_sfl->data) {
					last_sfl = last_sfl->next;
				}

				// Add the current node to the segregated free list
				new_sfl->next = last_sfl->next;
				new_sfl->prev = last_sfl;
				last_sfl->next = new_sfl;
			}

			// Update the number of free blocks in the list
			(*sfl_lists)[j].size += 1;

			return;
		}
	}

	// If there is no list with the remaining size, add a new list

	// Update the number of lists
	*lists_num += 1;
	*sfl_lists = realloc(*sfl_lists, *lists_num * sizeof(sfl_list_t));
	DIE(!*sfl_lists, "Realloc failed while reallocating sfl_lists");

	// Find the index of the new list
	for (size_t j = 0; j < *lists_num; j++) {
		if (j == *lists_num - 1 ||
		    (*sfl_lists)[j].element_size > block_size) {
			// Move the lists to the right
			for (size_t k = *lists_num - 1; k > j; k--)
				(*sfl_lists)[k] = (*sfl_lists)[k - 1];

			// Add and initialise the new list
			(*sfl_lists)[j].element_size = block_size;
			(*sfl_lists)[j].size = block_size;
			(*sfl_lists)[j].size = 1;
			(*sfl_lists)[j].head = new_sfl;
			new_sfl->next = NULL;
			new_sfl->prev = NULL;

			return;
		}
	}
}

void malloc_f(sfl_list_t **sfl_lists, size_t *lists_num,
	      ll_list_t *allocated_blocks, size_t *fragmentations,
	      size_t *malloc_calls)
{
	// Declare the variable for the block size
	size_t block_size;

	// Read the size of the block to be allocated
	scanf("%lu", &block_size);

	// Find the list with the smallest element size that can store the
	// requested size
	for (size_t i = 0; i < *lists_num; i++) {
		// If the current list is too small or empty, continue
		if ((*sfl_lists)[i].element_size < block_size ||
		    !(*sfl_lists)[i].head) {
			continue;
		}

		// Count valid malloc calls
		*malloc_calls += 1;

		// Calculate the remaining size
		size_t remaining_size =
			(*sfl_lists)[i].element_size - block_size;

		// Add a new node to the allocated blocks list and save the address
		size_t block_address = add_ll_node(sfl_lists, i, block_size,
						   allocated_blocks, lists_num);

		// Add the remaining memory to the next list
		if (remaining_size) {
			// Count fragmentations of the memory
			*fragmentations += 1;

			add_sfl_node(block_address + block_size, remaining_size,
				     sfl_lists, lists_num);
		}

		return;
	}

	// If there is no list with enough memory, print an error message
	printf("Out of memory\n");
}

ll_node_t *remove_ll_node(ll_list_t *allocated_blocks, size_t block_address,
			  size_t *free_calls, void *heap_data,
			  size_t start_address)
{
	// Find the block with the given address
	for (ll_node_t *current_ll = allocated_blocks->head; current_ll;
	     current_ll = current_ll->next) {
		if (block_address !=
		    (size_t)((char *)current_ll->data - (char *)heap_data +
			     start_address))
			continue;

		// Count the number of free calls for valid calls
		*free_calls += 1;

		// Remove the current node from the allocated blocks list
		if (current_ll->prev)
			current_ll->prev->next = current_ll->next;
		else
			allocated_blocks->head = current_ll->next;

		// Reconnect the allocated blocks list
		if (current_ll->next)
			current_ll->next->prev = current_ll->prev;

		// Update the number of allocated blocks
		allocated_blocks->size -= 1;

		// Check if the list is empty
		if (current_ll->size == 0)
			allocated_blocks->head = NULL;

		// Return the current node
		return current_ll;
	}

	return NULL;
}

void simple_free(ll_list_t *allocated_blocks, sfl_list_t **sfl_lists,
		 size_t block_address, size_t *lists_num, void *heap_data,
		 size_t start_address, size_t *free_calls)
{
	// Check if the address is NULL
	if (block_address == 0) {
		// Count free calls
		free_calls += 1;

		// Do nothing for free(NULL)
		return;
	}

	// Find the block with the given address
	ll_node_t *current_ll = remove_ll_node(allocated_blocks, block_address,
					       free_calls, heap_data,
					       start_address);

	// Add the current node to the segregated free list
	if (current_ll) {
		add_sfl_node((size_t)current_ll->data, current_ll->size,
			     sfl_lists, lists_num);
	} else { // If the address is not found, print an error message
		printf("Invalid free\n");
	}

	// Free the memory of the current node
	free(current_ll);
}

void free_f(sfl_list_t **sfl_lists, size_t *lists_num,
	    ll_list_t *allocated_blocks, size_t *free_calls,
	    size_t reconstruct_type, void *heap_data, size_t start_address)
{
	// Declare the variable for the block address
	size_t block_address;

	// Read the address of the block to be freed
	scanf("%lx", &block_address);

	if (reconstruct_type)
		return;

	// Free the block
	simple_free(allocated_blocks, sfl_lists, block_address, lists_num,
		    heap_data, start_address, free_calls);
}

bool read(ll_list_t allocated_blocks, void *heap_data, size_t start_address,
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
	for (ll_node_t *current = allocated_blocks.head; current;
	     current = current->next) {
		if (block_address ==
		    (size_t)current->data - (size_t)heap_data + start_address) {
			// Copy the data from the allocated memory to the text
			for (i = 0, read = 0;
			     i < read_size && i < current->size; i++, read++) {
				text[j++] = *((char *)current->data + i);
			}

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
	}

	// If the address is not found, print an error message
	printf("Segmentation fault (core dumped)\n");

	// Free the memory of the text
	free(text);

	// Dump the memory statistics
	dump_memory(lists_num, malloc_calls, fragmentations, free_calls,
		    sfl_lists, allocated_blocks, start_address, heap_data);

	// Destroy the heap
	destroy_heap(sfl_lists, lists_num, heap_data, allocated_blocks);

	// Free the command
	free(command);

	// Return false if the text is not read completely
	return false;
}

bool write(ll_list_t allocated_blocks, void *heap_data, size_t start_address,
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
	for (ll_node_t *current = allocated_blocks.head; current;
	     current = current->next) {
		if (block_address ==
		    (size_t)current->data - (size_t)heap_data + start_address) {
			// Copy the data from the text to the allocated memory
			for (i = 0; i < write_size && i < current->size &&
				    i < text_size;
			     i++, j++) {
				*((char *)current->data + i) = text[j];
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
	}

	// If the address is not found, print an error message
	printf("Segmentation fault (core dumped)\n");

	// Dump the memory statistics
	dump_memory(lists_num, malloc_calls, fragmentations, free_calls,
		    sfl_lists, allocated_blocks, start_address, heap_data);

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
		 ll_list_t allocated_blocks, size_t start_address,
		 void *heap_data)
{
	printf("+++++DUMP+++++\n");

	// Calculate the number of free blocks and the total free memory
	size_t free_blocks = 0;
	size_t free_memory = 0;
	for (size_t i = 0; i < lists_num; i++) {
		free_blocks += sfl_lists[i].size;
		free_memory += sfl_lists[i].size * sfl_lists[i].element_size;
	}

	// Calculate the total allocated memory
	size_t allocated_memory = 0;
	for (ll_node_t *current = allocated_blocks.head; current;
	     current = current->next) {
		allocated_memory += current->size;
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
		       sfl_lists[i].element_size, sfl_lists[i].size);

		// Print the addresses of the free blocks
		if (sfl_lists[i].head) {
			for (sfl_node_t *current = sfl_lists[i].head; current;
			     current = current->next) {
				printf("0x%lx", (size_t)current->data -
							(size_t)heap_data +
							start_address);

				// Print a space if there are more blocks
				if (current->next)
					printf(" ");
			}
		}

		printf("\n");
	}

	// Print the addresses of the allocated blocks
	printf("Allocated blocks :");
	if (allocated_blocks.head) {
		printf(" ");

		for (ll_node_t *current = allocated_blocks.head; current;
		     current = current->next) {
			printf("(0x%lx - %lu)",
			       (size_t)current->data - (size_t)heap_data +
				       start_address,
			       current->size);

			// Print a space if there are more blocks
			if (current->next)
				printf(" ");
		}
	}

	printf("\n-----DUMP-----\n");
}

void destroy_heap(sfl_list_t *sfl_lists, size_t lists_num, void *heap_data,
		  ll_list_t allocated_blocks)
{
	// Free the memory of the segregated free lists's nodes
	for (size_t i = 0; i < lists_num; i++) {
		sfl_node_t *current = sfl_lists[i].head;
		while (current) {
			sfl_node_t *next = current->next;
			free(current);
			current = next;
		}
	}

	// Free the memory of the allocated blocks nodes
	ll_node_t *current = allocated_blocks.head;
	while (current) {
		ll_node_t *next = current->next;
		free(current);
		current = next;
	}

	// Free the memory of the segregated free lists
	free(sfl_lists);

	// Free the memory of the heap
	free(heap_data);
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

			// Temporary fix for the reconstruct_type parameter
			if (reconstruct_type)
				return;

			// Initialize the heap
			sfl_lists = init_heap(start_address, lists_num,
					      bytes_per_list, &heap_data);
		} else if (!strcmp(command, "DUMP_MEMORY")) {
			// Dump the memory statistics
			dump_memory(lists_num, malloc_calls, fragmentations,
				    free_calls, sfl_lists, allocated_blocks,
				    start_address, heap_data);
		} else if (!strcmp(command, "MALLOC")) {
			malloc_f(&sfl_lists, &lists_num, &allocated_blocks,
				 &fragmentations, &malloc_calls);
		} else if (!strcmp(command, "FREE")) {
			free_f(&sfl_lists, &lists_num, &allocated_blocks,
			       &free_calls, reconstruct_type, heap_data,
			       start_address);

		} else if (!strcmp(command, "READ")) {
			if (!read(allocated_blocks, heap_data, start_address,
				  command, free_calls, fragmentations,
				  malloc_calls, sfl_lists, lists_num))
				return;

		} else if (!strcmp(command, "WRITE")) {
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
