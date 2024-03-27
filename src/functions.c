#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "utils.h"

// The size of the text from the input
#define TEXT_SIZE 600

sfl_list_t *init_heap(size_t heap_start, size_t num_lists,
		      size_t bytes_per_list, void **heap)
{
	// Allocate memory for the heap
	*heap = malloc(num_lists * bytes_per_list);
	DIE(*heap == NULL, "Malloc failed while allocating heap");

	// Set the starting address of the heap
	heap_start = (size_t)*heap;

	// Allocate memory for the segregated free lists
	sfl_list_t *lists = malloc(sizeof(sfl_list_t) * num_lists);
	DIE(lists == NULL, "Malloc failed while allocating lists");

	// Initialize each segregated free list
	for (size_t i = 0; i < num_lists; i++) {
		// Calculate the element size and size of the current list
		lists[i].element_size = 8 * (1 << i);

		lists[i].size = bytes_per_list / lists[i].element_size;

		// Create the head node for the current list
		sfl_node_t *previous = malloc(sizeof(sfl_node_t));
		DIE(previous == NULL,
		    "Malloc failed while allocating node for lists[i]");
		lists[i].head = previous;
		previous->data = (void *)(heap_start + i * bytes_per_list);
		previous->prev = NULL;

		// Create the remaining nodes for the current list
		for (size_t j = 1; j < lists[i].size; j++) {
			sfl_node_t *current = malloc(sizeof(sfl_node_t));
			DIE(current == NULL,
			    "Malloc failed while allocating node for lists[i]");

			// Set the data of the current node
			current->data =
				(void *)(heap_start + i * bytes_per_list +
					 j * lists[i].element_size);

			// Connect the current node to the previous one
			previous->next = current;
			current->prev = previous;

			// Move
			previous = current;
		}

		// Set the next pointer of the last node to NULL
		previous->next = NULL;
	}

	return lists;
}

void malloc_f(size_t size, sfl_list_t **lists, size_t *num_lists,
	      ll_list_t *allocated_blocks, size_t *fragmentations,
	      size_t *malloc_calls)
{
	// Find the list with the smallest element size that can store the
	// requested size
	for (size_t i = 0; i < *num_lists; i++) {
		// If the current list is too small or empty, continue
		if ((*lists)[i].element_size < size ||
		    (*lists)[i].head == NULL) {
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

		ll_node_t *last_ll = allocated_blocks->head;
		if (last_ll == NULL) {
			allocated_blocks->head = new_ll;
		} else if (new_ll->data < last_ll->data) {
			last_ll->prev = new_ll;
			new_ll->next = last_ll;

			allocated_blocks->head = new_ll;
		} else {
			// Move to the appropriate position
			while (last_ll->next != NULL &&
			       last_ll->next->data < new_ll->data) {
				last_ll = last_ll->next;
			}

			// Add the current node to the allocated blocks list
			new_ll->next = last_ll->next;
			new_ll->prev = last_ll;

			if (last_ll->next) {
				last_ll->next->prev = new_ll;
			}
			last_ll->next = new_ll;
		}

		// Update the number of allocated blocks
		allocated_blocks->size += 1;

		// Remove the first node from the segregated free list
		sfl_node_t *first_sfl = (*lists)[i].head;
		(*lists)[i].head = (*lists)[i].head->next;
		if ((*lists)[i].head) {
			(*lists)[i].head->prev = NULL;
		}

		// Update the number of free blocks in the list
		(*lists)[i].size -= 1;

		// Calculate the remaining size
		size_t remaining_size = (*lists)[i].element_size - size;

		// Check if the list is empty
		if ((*lists)[i].size == 0) {
			// Update the number of lists
			*num_lists -= 1;

			// Move the lists to the left
			for (size_t j = i; j < *num_lists; j++) {
				(*lists)[j] = (*lists)[j + 1];
			}

			// Reallocate memory for the segregated free lists
			*lists = realloc(*lists,
					 *num_lists * sizeof(sfl_list_t));
			DIE(lists == NULL,
			    "Realloc failed while reallocating lists");
		}

		// Free the memory of the removed node
		free(first_sfl);

		// Add the remaining memory to the next list
		if (!remaining_size) {
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

			// Find the list which mathces the remaining size
			for (size_t j = 0; j < *num_lists; j++) {
				if ((*lists)[j].element_size ==
				    remaining_size) {
					sfl_node_t *last_sfl = (*lists)[j].head;
					if (new_sfl->data < last_sfl->data) {
						last_sfl->prev = new_sfl;
						new_sfl->next = last_sfl;

						(*lists)[j].head = new_sfl;
					} else {
						// Move to the appropriate position
						while (last_sfl->next != NULL &&
						       last_sfl->next->data <
							       new_sfl->data) {
							last_sfl =
								last_sfl->next;
						}

						// Add the current node to the segregated free list
						new_sfl->next = last_sfl->next;
						new_sfl->prev = last_sfl;
						last_sfl->next = new_sfl;
					}

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
				if (j == *num_lists - 1 ||
				    (*lists)[j].element_size > remaining_size) {
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

void simple_free(size_t address, ll_list_t *allocated_blocks,
		 sfl_list_t **lists, size_t *num_lists, void *heap,
		 size_t start_address, size_t *free_calls)
{
	// Check if the address is NULL
	if (address == 0) {
		// Count free calls
		*free_calls += 1;

		// Do nothing for free(NULL)
		return;
	}

	// Find the block with the given address
	for (ll_node_t *current_ll = allocated_blocks->head; current_ll;
	     current_ll = current_ll->next) {
		if (address == (size_t)((char *)current_ll->data -
					(char *)heap + start_address)) {
			// Count the number of free calls for valid calls
			*free_calls += 1;

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

			if (current_ll->size == 0) {
				allocated_blocks->head = NULL;
			}

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

					// Check if the new node is the first one
					if (new_sfl->data <
					    (*lists)[i].head->data) {
						// Connect the new node to the first one
						new_sfl->next =
							(*lists)[i].head;
						new_sfl->prev = NULL;
						(*lists)[i].head->prev =
							new_sfl;
						(*lists)[i].head = new_sfl;
					} else {
						// Move to the appropriate position
						sfl_node_t *previous_sfl =
							(*lists)[i].head;
						while (previous_sfl->next !=
							       NULL &&
						       previous_sfl->data <
							       new_sfl->data) {
							previous_sfl =
								previous_sfl
									->next;
						}

						// Connect the new node to the last one
						new_sfl->prev = previous_sfl;
						new_sfl->next =
							previous_sfl->next;
						previous_sfl->next = new_sfl;
					}

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
			DIE(lists == NULL,
			    "Realloc failed while reallocating lists");

			// Find the index of the new list
			for (size_t i = 0; i < *num_lists; i++) {
				if (i == *num_lists - 1 ||
				    (*lists)[i].element_size >
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

bool read(ll_list_t *allocated_blocks, size_t address, size_t size, void *heap,
	  size_t start_address)
{
	// Allocate memory for the block
	char *block = malloc(size + 1);
	DIE(block == NULL, "Malloc failed while allocating block");

	// Make an index for the block
	size_t j = 0;

	// Counter for the number of bytes read
	size_t read = 0;

	// Make an index for the allocated block
	size_t i = 0;

	// Find the block with the given address
	for (ll_node_t *current = allocated_blocks->head; current != NULL;
	     current = current->next) {
		if (address ==
		    (size_t)current->data - (size_t)heap + start_address) {
			// Copy the data from the block to the allocated memory
			for (i = 0, read = 0; i < size && i < current->size;
			     i++, read++) {
				block[j++] = *((char *)current->data + i);
			}

			// Update the size and address
			size -= read;
			address += read;

			// Check if the block is read completely
			if (size == 0) {
				// Add the null terminator
				block[j] = '\0';

				// Print the block
				printf("%s\n", block);

				// Free the memory of the block
				free(block);

				// Return true if the block is read completely
				return true;
			}
		}
	}

	// If the address is not found, print an error message
	printf("Segmentation fault (core dumped)\n");

	// Free the memory of the block
	free(block);

	// Return false if the block is not read completely
	return false;
}

bool write(ll_list_t *allocated_blocks, size_t address, size_t size, void *heap,
	   size_t start_address, char *block)
{
	// Calculate the original block size
	size_t block_size = strlen(block);

	// Make an index for the allocated block
	size_t i;

	// Make an index for the soon-to-be-written block
	size_t j = 0;

	// Find the block with the given address
	for (ll_node_t *current = allocated_blocks->head; current != NULL;
	     current = current->next) {
		if (address ==
		    (size_t)current->data - (size_t)heap + start_address) {
			// Copy the data from the block to the allocated memory
			for (i = 0;
			     i < size && i < current->size && i < block_size;
			     i++, j++) {
				*((char *)current->data + i) = block[j];
			}

			// Update the size, address and block size
			size -= i;
			block_size -= i;
			address += i;

			// Check if the block is written completely and return true
			if (size == 0 || block_size == 0) {
				return true;
			}
		}
	}

	// If the address is not found, print an error message
	printf("Segmentation fault (core dumped)\n");

	// Return false if the block is not written completely
	return false;
}

void dump_memory(size_t num_lists, size_t malloc_calls, size_t fragmentations,
		 size_t free_calls, sfl_list_t *lists,
		 ll_list_t allocated_blocks, size_t start_address, void *data)
{
	printf("+++++DUMP+++++\n");

	// Calculate the number of free blocks
	size_t free_blocks = 0;
	for (size_t i = 0; i < num_lists; i++) {
		free_blocks += lists[i].size;
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
		free_memory += lists[i].size * lists[i].element_size;
	}

	// Print the total memory, total allocated memory, total free memory,
	// number of free blocks, number of allocated blocks, number of malloc
	// calls, number of fragmentations, and number of free calls

	printf("Total memory: %lu bytes\n", free_memory + allocated_memory);
	printf("Total allocated memory: %lu bytes\n", allocated_memory);
	printf("Total free memory: %lu bytes\n", free_memory);
	printf("Free blocks: %lu\n", free_blocks);

	// Formula: (bytes_per_list / 8) * ((2^num_lists - 1) / 2^(num_lists - 1))
	printf("Number of allocated blocks: %lu\n", malloc_calls - free_calls);

	printf("Number of malloc calls: %lu\n", malloc_calls);
	printf("Number of fragmentations: %lu\n", fragmentations);
	printf("Number of free calls: %lu\n", free_calls);

	// Print blocks with their respective sizes and number of free blocks
	for (size_t i = 0; i < num_lists; i++) {
		printf("Blocks with %lu bytes - %lu free block(s) : ",
		       lists[i].element_size, lists[i].size);

		// Print the addresses of the free blocks
		if (lists[i].head) {
			for (sfl_node_t *current = lists[i].head;
			     current != NULL; current = current->next) {
				printf("0x%lx", (size_t)current->data -
							(size_t)data +
							start_address);

				// Print a space if there are more blocks
				if (current->next) {
					printf(" ");
				}
			}
		}

		printf("\n");
	}

	// Print the addresses of the allocated blocks
	printf("Allocated blocks :");
	if (allocated_blocks.head) {
		printf(" ");

		for (ll_node_t *current = allocated_blocks.head;
		     current != NULL; current = current->next) {
			printf("(0x%lx - %lu)",
			       (size_t)current->data - (size_t)data +
				       start_address,
			       current->size);

			// Print a space if there are more blocks
			if (current->next) {
				printf(" ");
			}
		}
	}

	printf("\n-----DUMP-----\n");
}

void destroy_heap(sfl_list_t *lists, size_t num_lists, void *data,
		  ll_list_t allocated_blocks)
{
	// Free the memory of the segregated free lists nodes
	for (size_t i = 0; i < num_lists; i++) {
		sfl_node_t *current = lists[i].head;
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
	free(lists);

	// Free the memory of the heap
	free(data);
}

char *read_block(void)
{
	// Allocate memory for the block
	char *block = malloc(TEXT_SIZE * sizeof(char));
	DIE(block == NULL, "Malloc failed while allocating block");

	// Initialize an index for the block
	size_t j = 0;

	char c = fgetc(stdin);

	// Skip characters until the opening quotation mark is found
	do {
		c = fgetc(stdin);
	} while (c != '"');

	// Read characters until the closing quotation mark is found
	do {
		c = fgetc(stdin);
		block[j++] = c;
	} while (c != '"');

	// Add the null terminator
	block[j - 1] = '\0';

	return block;
}
