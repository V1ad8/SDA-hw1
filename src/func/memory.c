#include "../header.h"

void malloc_f(sfl_list_t **sfl_lists, size_t *lists_num,
			  sfl_list_t *allocated_blocks, size_t *fragmentations,
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
		if (((block_t *)(*sfl_lists)[i].head->data)->size < block_size ||
			!(*sfl_lists)[i].head)
			continue;

		// Count valid malloc calls
		*malloc_calls += 1;

		// Calculate the remaining size
		size_t remaining_size =
			((block_t *)(*sfl_lists)[i].head->data)->size - block_size;

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

		// Return if the block was successfully allocated
		return;
	}

	// If there is no list with enough memory, print an error message
	printf("Out of memory\n");
}

bool defragmented(size_t *lists_num, sfl_list_t **sfl_lists,
				  size_t *block_address, size_t *block_size, void *heap_data,
				  size_t start_address, size_t bytes_per_list)
{
	// Search for compatible blocks in the segregated free lists
	for (size_t i = 0; i < *lists_num; i++) {
		for (sfl_node_t *current = (*sfl_lists)[i].head; current;
			 current = current->next) {
			// Check if the current node has the same parent block as the
			// freed block
			if (!same_parent(*block_address - start_address,
							 (size_t)((block_t *)(size_t)current->data)
							 ->address,
							 heap_data, bytes_per_list))
				continue;

			// Check if the current node is adjacent to the freed block
			if ((size_t)((block_t *)(size_t)current->data)->address -
						(size_t)heap_data + start_address +
						((block_t *)(*sfl_lists)[i].head->data)->size !=
					*block_address &&
				(size_t)((block_t *)(size_t)current->data)->address -
						(size_t)heap_data + start_address - *block_size !=
					*block_address)
				continue;

			// If the current node is adjacent to the freed block, merge them
			if ((size_t)((block_t *)(size_t)current->data)->address -
					(size_t)heap_data + start_address +
					((block_t *)(*sfl_lists)[i].head->data)->size ==
				*block_address)
				*block_address -= ((block_t *)(*sfl_lists)[i].head->data)->size;

			*block_size += ((block_t *)(*sfl_lists)[i].head->data)->size;

			// Remove the current node from the segregated free list
			if ((*sfl_lists)[i].head != current && current->prev)
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

			// Free the memory of the removed node
			free(current->data);
			free(current);

			// Return true if the blocks were merged
			return true;
		}
	}

	// Return false if the blocks were not merged
	return false;
}

void free_f(sfl_list_t **sfl_lists, size_t *lists_num,
			sfl_list_t *allocated_blocks, size_t *free_calls,
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

	// Find the block in the allocated blocks list
	sfl_node_t *current_ll = remove_ll_node(allocated_blocks, block_address,
											heap_data, start_address);
	if (!current_ll) {
		// Print an error message if the block was not found
		printf("Invalid free\n");
		return;
	}

	// Count free calls
	*free_calls += 1;

	// Save the block size so it can be increased if the block is merged
	size_t block_size = ((block_t *)current_ll->data)->size;

	bool loop = true;
	if (reconstruct_type)
		while (loop)
			loop = defragmented(lists_num, sfl_lists, &block_address,
								&block_size, heap_data, start_address,
								bytes_per_list);

	// Free the block
	add_sfl_node(block_address + (size_t)heap_data - start_address, block_size,
				 sfl_lists, lists_num);

	// Free the memory of the removed node
	free(current_ll->data);
	free(current_ll);
}
