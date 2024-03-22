#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

// The size of the command from the input
#define COMMAND_SIZE 100

// Structure for a node in the segregated free list
typedef struct sfl_node_t {
	void *data;
	struct sfl_node_t *next, *prev;
} sfl_node_t;

// Structure for a segregated free list
typedef struct sfl_list_t {
	sfl_node_t *head;
	size_t size;
	size_t element_size; // The size of the elements in the list
} sfl_list_t;

// Structure for a node in the linked list
typedef struct ll_node_t {
	void *data;
	size_t size; // The number of elements stored in the current block
	struct ll_node_t *next, *prev;
} ll_node_t;

// Structure for a linked list
typedef struct ll_list_t {
	ll_node_t *head;
	size_t size;
} ll_list_t;

// Function to initialize the heap
//
// Parameters:
//	 - heap_start: The starting address of the heap
//	 - num_lists: The number of segregated free lists
//	 - bytes_per_list: The number of bytes per list
//	 - data: Pointer to store the allocated memory for the heap
//
// Returns:
//	 - The array of segregated free lists
sfl_list_t *init_heap(size_t heap_start, size_t num_lists,
		      size_t bytes_per_list, void **data)
{
	// Allocate memory for the heap
	*data = malloc(num_lists * bytes_per_list);
	DIE(*data == NULL, "Malloc failed while allocating heap");

	// Set the starting address of the heap
	heap_start = (size_t)*data;

	// Allocate memory for the segregated free lists
	sfl_list_t *segregated_free_lists =
		malloc(sizeof(sfl_list_t) * num_lists);
	DIE(segregated_free_lists == NULL,
	    "Malloc failed while allocating segregated_free_lists");

	// Initialize each segregated free list
	for (size_t i = 0; i < num_lists; i++) {
		// Calculate the element size and size of the current list
		segregated_free_lists[i].element_size = 8 * (1 << i);

		segregated_free_lists[i].size =
			bytes_per_list / segregated_free_lists[i].element_size;

		// Create the head node for the current list
		sfl_node_t *previous = malloc(sizeof(sfl_node_t));
		DIE(previous == NULL,
		    "Malloc failed while allocating node for segregated_free_lists[i]");
		segregated_free_lists[i].head = previous;
		previous->data = (void *)(heap_start + i * bytes_per_list);
		previous->prev = NULL;

		// Create the remaining nodes for the current list
		for (size_t j = 1; j < segregated_free_lists[i].size; j++) {
			sfl_node_t *current = malloc(sizeof(sfl_node_t));
			DIE(current == NULL,
			    "Malloc failed while allocating node for segregated_free_lists[i]");

			// Set the data of the current node
			current->data =
				(void *)(heap_start + i * bytes_per_list +
					 j * segregated_free_lists[i]
							 .element_size);

			// Connect the current node to the previous one
			previous->next = current;
			current->prev = previous;

			// Move
			previous = current;
		}

		// Set the next pointer of the last node to NULL
		previous->next = NULL;
	}

	return segregated_free_lists;
}

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
	      size_t *malloc_calls)
{
	// Find the list with the smallest element size that can store the
	// requested size
	for (size_t i = 0; i < *num_lists; i++) {
		// If the current list is too small, continue
		if ((*lists)[i].element_size < size) {
			continue;
		}

		// Count valid malloc calls
		*malloc_calls += 1;

		// Allocate memory for a new node in the allocated blocks list
		ll_node_t *new_ll = malloc(sizeof(ll_node_t));
		DIE(new_ll == NULL, "Malloc failed while allocating node");

		// Initialise the data of the current node
		new_ll->data = (*lists)[i].head->data;
		new_ll->size = size;
		new_ll->next = NULL;
		new_ll->prev = NULL;

		// Move to the end of the list
		ll_node_t *last_ll = allocated_blocks->head;
		if (last_ll == NULL) {
			allocated_blocks->head = new_ll;
		} else {
			while (last_ll->next != NULL) {
				last_ll = last_ll->next;
			}

			// Add the current node to the allocated blocks list
			last_ll->next = new_ll;
			new_ll->prev = last_ll;
			new_ll->next = NULL;
		}

		// Update the number of allocated blocks
		allocated_blocks->size += 1;

		// Remove the first node from the segregated free list
		sfl_node_t *first_sfl = (*lists)[i].head;
		(*lists)[i].head = first_sfl->next;
		(*lists)[i].head->prev = NULL;
		(*lists)[i].size -= 1;

		// Free the memory of the removed node
		free(first_sfl);

		// Add the remaining memory to the next list
		if ((*lists)[i].element_size == size) {
			return;
		} else {
			// Count fragmentations
			*fragmentations += 1;

			// Allocate memory for a new node in the segregated free list
			sfl_node_t *new_sfl = malloc(sizeof(sfl_node_t));
			DIE(new_sfl == NULL,
			    "Malloc failed while allocating node");

			// Set the data of the current node
			new_sfl->data = (void *)((size_t)new_ll->data + size);

			// Calculate the remaining memory
			size_t remaining_size = (*lists)[i].element_size - size;

			// Find the list which mathces the remaining size
			for (size_t j = 0; j < *num_lists; j++) {
				if ((*lists)[j].element_size ==
				    remaining_size) {
					// Move to the end of the list
					sfl_node_t *last_sfl = (*lists)[j].head;
					while (last_sfl->next != NULL) {
						last_sfl = last_sfl->next;
					}

					// Connect the new node to the last one
					last_sfl->next = new_sfl;
					new_sfl->prev = last_sfl;
					new_sfl->next = NULL;

					// Update the number of free blocks in the list
					(*lists)[j].size += 1;

					return;
				}
			}

			// If there is no list with the remaining size, add a new list

			// Update the number of lists
			*num_lists += 1;
			*lists = realloc(*lists,
					 *num_lists * sizeof(sfl_list_t));
			DIE(*lists == NULL,
			    "Realloc failed while reallocating lists");

			// Find the index of the new list
			for (size_t j = 0; j < *num_lists; j++) {
				if ((*lists)[j].element_size > remaining_size) {
					// Move the lists to the right
					for (size_t k = *num_lists - 1; k > j;
					     k--) {
						(*lists)[k] = (*lists)[k - 1];
					}

					// Add and initialise the new list
					(*lists)[j].element_size =
						remaining_size;
					(*lists)[j].size = remaining_size;
					(*lists)[j].size = 1;
					(*lists)[j].head = new_sfl;
					new_sfl->next = NULL;
					new_sfl->prev = NULL;

					return;
				}
			}
		}
	}

	// If there is no list with enough memory, print an error message
	printf("Out of memory\n");
}

// Function to free memory using segregated free lists
//
// Parameters:
//   - address: The address of the block to be freed
//   - allocated_blocks: Pointer to the linked list of allocated blocks
//   - lists: Pointer to the array of segregated free lists
//   - num_lists: Pointer to the number of segregated free lists
//   - data: Pointer to the allocated memory for the heap
//   - start_address: The starting address of the heap
void simple_free(size_t address, ll_list_t *allocated_blocks,
		 sfl_list_t **lists, size_t *num_lists, void *data,
		 size_t start_address)
{
	// Check if the address is NULL
	if (address == 0) {
		return;
	}

	// Find the block with the given address
	for (ll_node_t *current_ll = allocated_blocks->head; current_ll;
	     current_ll = current_ll->next) {
		if (address == (size_t)((char *)current_ll->data -
					(char *)data + start_address)) {
			// Remove the current node from the allocated blocks list
			if (current_ll->prev) {
				current_ll->prev->next = current_ll->next;
			} else {
				allocated_blocks->head = current_ll->next;
			}

			// Reconnect the allocated blocks list
			if (current_ll->next) {
				current_ll->next->prev = current_ll->prev;
			}

			// Update the number of allocated blocks
			allocated_blocks->size -= 1;

			// Add the current node to the segregated free list
			for (size_t i = 0; i < *num_lists; i++) {
				if (current_ll->size ==
				    (*lists)[i].element_size) {
					// Allocate memory for a new node in the segregated free list
					sfl_node_t *new_sfl =
						malloc(sizeof(sfl_node_t));
					DIE(new_sfl == NULL,
					    "Malloc failed while allocating node");

					// Set the data of the current node
					new_sfl->data = current_ll->data;

					// Move to the appropriate position
					sfl_node_t *previous_sfl =
						(*lists)[i].head;
					while (previous_sfl->next != NULL &&
					       previous_sfl->data <
						       new_sfl->data) {
						previous_sfl =
							previous_sfl->next;
					}

					// Connect the new node to the last one
					new_sfl->prev = previous_sfl;
					new_sfl->next = previous_sfl->next;
					previous_sfl->next = new_sfl;

					// Update the number of free blocks in the list
					(*lists)[i].size += 1;

					// Free the memory of the freed node
					free(current_ll);
					return;
				}
			}

			// If there is no list with the remaining size, add a new list

			// Update the number of lists
			*num_lists += 1;

			// Reallocate memory for the segregated free lists
			*lists = realloc(*lists,
					 *num_lists * sizeof(sfl_list_t));

			// Find the index of the new list
			for (size_t i = 0; i < *num_lists; i++) {
				if ((*lists)[i].element_size >
				    current_ll->size) {
					// Move the lists to the right
					for (size_t j = *num_lists - 1; j > i;
					     j--) {
						(*lists)[j] = (*lists)[j - 1];
					}

					// Add and initialise the new list
					(*lists)[i].element_size =
						current_ll->size;
					(*lists)[i].size = 1;
					(*lists)[i].head =
						malloc(sizeof(sfl_node_t));
					(*lists)[i].head->data =
						current_ll->data;
					(*lists)[i].head->next = NULL;
					(*lists)[i].head->prev = NULL;

					// Free the memory of the freed node
					free(current_ll);
					return;
				}
			}
		}
	}

	// If the address is not found, print an error message
	printf("Invalid free\n");
}

// Function to dump the memory statistics
//
// Parameters:
//   - num_lists: The number of segregated free lists
//   - malloc_calls: The number of malloc calls
//   - fragmentations: The number of fragmentations
//   - free_calls: The number of free calls
//   - segregated_free_lists: The array of segregated free lists
//   - allocated_blocks: The linked list of allocated blocks
//   - start_address: The starting address of the heap
//   - data: Pointer to the allocated memory for the heap
void dump_memory(size_t num_lists, size_t malloc_calls, size_t fragmentations,
		 size_t free_calls, sfl_list_t *segregated_free_lists,
		 ll_list_t allocated_blocks, size_t start_address, void *data)
{
	printf("+++++DUMP+++++\n");

	// Calculate the number of free blocks
	size_t free_blocks = 0;
	for (size_t i = 0; i < num_lists; i++) {
		free_blocks += segregated_free_lists[i].size;
	}

	// Calculate the total allocated memory
	size_t allocated_memory = 0;
	for (ll_node_t *current = allocated_blocks.head; current != NULL;
	     current = current->next) {
		allocated_memory += current->size;
	}

	// Calculate the total free memory
	size_t free_memory = 0;
	for (size_t i = 0; i < num_lists; i++) {
		free_memory += segregated_free_lists[i].size *
			       segregated_free_lists[i].element_size;
	}

	// Print the total memory, total allocated memory, total free memory,
	// number of free blocks, number of allocated blocks, number of malloc
	// calls, number of fragmentations, and number of free calls

	printf("Total memory: %lu bytes\n", free_memory + allocated_memory);
	printf("Total allocated memory: %lu bytes\n", allocated_memory);
	printf("Total free memory: %lu bytes\n", free_memory);
	printf("Number of free blocks: %lu\n", free_blocks);

	// Formula: (bytes_per_list / 8) * ((2^num_lists - 1) / 2^(num_lists - 1))
	printf("Number of allocated blocks: %lu\n", malloc_calls - free_calls);

	printf("Number of malloc calls: %lu\n", malloc_calls);
	printf("Number of fragmentations: %lu\n", fragmentations);
	printf("Number of free calls: %lu\n", free_calls);

	// Print blocks with their respective sizes and number of free blocks
	for (size_t i = 0; i < num_lists; i++) {
		printf("Blocks with %lu bytes - %lu free block(s) : ",
		       segregated_free_lists[i].element_size,
		       segregated_free_lists[i].size);

		// Print the addresses of the free blocks
		if (segregated_free_lists[i].head) {
			for (sfl_node_t *current =
				     segregated_free_lists[i].head;
			     current != NULL; current = current->next) {
				printf("0x%lx ", (size_t)current->data -
							 (size_t)data +
							 start_address);
			}
		}

		printf("\n");
	}

	// Print the addresses of the allocated blocks
	printf("Allocated blocks : ");
	if (allocated_blocks.head) {
		for (ll_node_t *current = allocated_blocks.head;
		     current != NULL; current = current->next) {
			printf("(0x%lx - %lu) ",
			       (size_t)current->data - (size_t)data +
				       start_address,
			       current->size);
		}
	}

	printf("\n-----DUMP-----\n");
}

// Function to free the memory of the heap
//
// Parameters:
//   - segregated_free_lists: The array of segregated free lists
//   - num_lists: The number of segregated free lists
//   - data: Pointer to the allocated memory for the heap
//   - allocated_blocks: Linked list of allocated blocks
void destroy_heap(sfl_list_t *segregated_free_lists, size_t num_lists,
		  void *data, ll_list_t allocated_blocks)
{
	// Free the memory of the segregated free lists nodes
	for (size_t i = 0; i < num_lists; i++) {
		sfl_node_t *current = segregated_free_lists[i].head;
		while (current != NULL) {
			sfl_node_t *next = current->next;
			free(current);
			current = next;
		}
	}

	// Free the memory of the allocated blocks nodes
	ll_node_t *current = allocated_blocks.head;
	while (current != NULL) {
		ll_node_t *next = current->next;
		free(current);
		current = next;
	}

	// Free the memory of the segregated free lists
	free(segregated_free_lists);

	// Free the memory of the heap
	free(data);
}

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
	size_t start_address, num_lists, bytes_per_list, type, size, address;

	// Allocate memory for the command
	char command[COMMAND_SIZE];

	while (1) {
		// Read the command from the input
		scanf("%s", command);

		if (!strcmp(command, "INIT_HEAP")) {
			// Read the parameters for the INIT_HEAP command
			scanf("%lx %lu %lu %lu", &start_address, &num_lists,
			      &bytes_per_list, &type);

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

			simple_free(address, &allocated_blocks, &list,
				    &num_lists, data, start_address);
			free_calls += 1;
		} else if (!strcmp(command, "DESTROY_HEAP")) {
			// Destroy the heap
			destroy_heap(list, num_lists, data, allocated_blocks);

			// Exit the program
			return 0;
		}
	}
}