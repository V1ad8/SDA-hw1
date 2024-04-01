#include "../header.h"

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
		size_t remaining_size = (*sfl_lists)[i].element_size - block_size;

		// Add a new node to the allocated blocks list and save the address
		size_t block_address =
			add_ll_node(sfl_lists, i, block_size, allocated_blocks, lists_num);

		// Add the remaining memory to the next list
		if (remaining_size) {
			// Count fragmentations of the memory
			*fragmentations += 1;

			add_sfl_node(block_address + block_size, remaining_size, sfl_lists,
						 lists_num);
		}

		return;
	}

	// If there is no list with enough memory, print an error message
	printf("Out of memory\n");
}

bool same_parent(size_t first_address, size_t second_address, void *heap_data,
				 size_t bytes_per_list)
{
	// first_address -= (size_t)heap_data;
	second_address -= (size_t)heap_data;

	if (first_address / bytes_per_list != second_address / bytes_per_list) {
		return false;
	}

	size_t parent = first_address / bytes_per_list;
	size_t size = 8 * (1 << parent);

	first_address = first_address % bytes_per_list;
	second_address = second_address % bytes_per_list;

	return (first_address / size == second_address / size);
}

bool defragmented(size_t *lists_num, sfl_list_t **sfl_lists,
				  size_t *block_address, size_t *block_size, void *heap_data,
				  size_t start_address, size_t bytes_per_list)
{
	for (size_t i = 0; i < *lists_num; i++) {
		for (sfl_node_t *current = (*sfl_lists)[i].head; current;
			 current = current->next) {
			if (!same_parent(*block_address - start_address,
							 (size_t)current->data, heap_data, bytes_per_list))
				continue;

			if ((size_t)current->data - (size_t)heap_data + start_address +
						(*sfl_lists)[i].element_size !=
					*block_address &&
				(size_t)current->data - (size_t)heap_data + start_address -
						*block_size !=
					*block_address)
				continue;

			if ((size_t)current->data - (size_t)heap_data + start_address +
					(*sfl_lists)[i].element_size ==
				*block_address)
				*block_address -= (*sfl_lists)[i].element_size;

			*block_size += (*sfl_lists)[i].element_size;

			// Remove the current node from the segregated free list
			if (current->prev)
				current->prev->next = current->next;
			else
				(*sfl_lists)[i].head = current->next;

			// Reconnect the segregated free list
			if (current->next)
				current->next->prev = current->prev;

			// Update the number of free blocks in the list
			(*sfl_lists)[i].size -= 1;

			// Check if the list is empty
			if ((*sfl_lists)[i].size == 0) {
				// Update the number of lists
				*lists_num -= 1;

				// Move the lists to the left
				for (size_t j = i; j < *lists_num; j++)
					(*sfl_lists)[j] = (*sfl_lists)[j + 1];

				// Reallocate memory for the segregated free lists
				*sfl_lists =
					realloc(*sfl_lists, *lists_num * sizeof(sfl_list_t));
				DIE(!sfl_lists, "Realloc failed while reallocating sfl_lists");
			}

			free(current);

			return true;
		}
	}

	return false;
}

void free_f(sfl_list_t **sfl_lists, size_t *lists_num,
			ll_list_t *allocated_blocks, size_t *free_calls,
			size_t reconstruct_type, void *heap_data, size_t start_address,
			size_t bytes_per_list)
{
	// Declare the variable for the block address
	size_t block_address;

	// Read the address of the block to be freed
	scanf("%lx", &block_address);

	// size_t original_address = block_address;

	if (block_address == 0) {
		// Count free calls
		*free_calls += 1;

		// Do nothing for free(NULL)
		return;
	}

	ll_node_t *current_ll = remove_ll_node(allocated_blocks, block_address,
										   heap_data, start_address);
	if (!current_ll) {
		printf("Invalid free\n");
		return;
	}

	// Count free calls
	*free_calls += 1;

	size_t block_size = current_ll->size;

	bool loop = true;
	if (reconstruct_type)
		while (loop)
			loop = defragmented(lists_num, sfl_lists, &block_address,
								&block_size, heap_data, start_address,
								bytes_per_list);

	// Free the block
	add_sfl_node(block_address + (size_t)heap_data - start_address, block_size,
				 sfl_lists, lists_num);

	free(current_ll);
}
