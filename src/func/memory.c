#include "../header.h"

void add_sfl_node(size_t block_address, size_t block_size,
				  sfl_list_t **sfl_lists, size_t *lists_num, size_t parent_id)
{
	// Allocate memory for a new node in the segregated free list
	sfl_node_t *new_sfl = malloc(sizeof(sfl_node_t));
	DIE(!new_sfl, "Malloc failed while allocating node");

	// Set the data of the current node
	new_sfl->data = (void *)(block_address);
	new_sfl->index = parent_id;

	new_sfl->next = NULL;
	new_sfl->prev = NULL;

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
				while (last_sfl->next && last_sfl->next->data < new_sfl->data) {
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
		if (j == *lists_num - 1 || (*sfl_lists)[j].element_size > block_size) {
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
		size_t remaining_size = (*sfl_lists)[i].element_size - block_size;

		size_t parent_id = (*sfl_lists)->head->index;

		// Add a new node to the allocated blocks list and save the address
		size_t block_address =
			add_ll_node(sfl_lists, i, block_size, allocated_blocks, lists_num);

		// Add the remaining memory to the next list
		if (remaining_size) {
			// Count fragmentations of the memory
			*fragmentations += 1;

			add_sfl_node(block_address + block_size, remaining_size, sfl_lists,
						 lists_num, parent_id);
		}

		return;
	}

	// If there is no list with enough memory, print an error message
	printf("Out of memory\n");
}

bool defragmented(size_t *lists_num, sfl_list_t **sfl_lists,
				  size_t *block_address, size_t *block_size, void *heap_data,
				  size_t start_address, size_t parent_id)
{
	for (size_t i = 0; i < *lists_num; i++) {
		for (sfl_node_t *current = (*sfl_lists)[i].head; current;
			 current = current->next) {
			if (current->index != parent_id)
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
			if (current && current->prev)
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
			size_t reconstruct_type, void *heap_data, size_t start_address)
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
								current_ll->parent_id);

	// Free the block
	add_sfl_node(block_address + (size_t)heap_data - start_address, block_size,
				 sfl_lists, lists_num, current_ll->parent_id);

	free(current_ll);
}
