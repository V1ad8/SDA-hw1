#include "../header.h"

size_t add_ll_node(list_t **sfl_lists, size_t index, size_t block_size,
				   list_t *allocated_blocks, size_t *lists_num)
{
	// Allocate memory for a new node in the allocated blocks list
	node_t *new_ll = malloc(sizeof(node_t));
	DIE(!new_ll, "Malloc failed while allocating node");

	new_ll->data = malloc(sizeof(block_t));
	DIE(!new_ll->data, "Malloc failed while allocating data for new_ll");

	// Initialise the data of the current node
	((block_t *)new_ll->data)->address =
		((block_t *)(*sfl_lists)[index].head->data)->address;
	((block_t *)new_ll->data)->size = block_size;
	new_ll->next = NULL;
	new_ll->prev = NULL;

	// Find the position of the new node in the allocated blocks list
	node_t *last_ll = allocated_blocks->head;
	if (!last_ll) {
		allocated_blocks->head = new_ll;
	} else if (((block_t *)new_ll->data)->address <
			   ((block_t *)last_ll->data)->address) {
		last_ll->prev = new_ll;
		new_ll->next = last_ll;

		allocated_blocks->head = new_ll;
	} else {
		// Move to the appropriate position
		while (last_ll->next && ((block_t *)last_ll->next->data)->address <
									((block_t *)new_ll->data)->address)
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
	node_t *first_sfl = (*sfl_lists)[index].head;
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
		*sfl_lists = realloc(*sfl_lists, *lists_num * sizeof(list_t));
		DIE(!sfl_lists, "Realloc failed while reallocating sfl_lists");
	}

	// Free the memory of the removed node
	free(first_sfl->data);
	free(first_sfl);

	// Return the address of the allocated block
	return (size_t)((block_t *)new_ll->data)->address;
}

void add_sfl_node(size_t block_address, size_t block_size, list_t **sfl_lists,
				  size_t *lists_num)
{
	// Allocate memory for a new node in the segregated free list
	node_t *new_sfl = malloc(sizeof(node_t));
	DIE(!new_sfl, "Malloc failed while allocating node");

	new_sfl->data = malloc(sizeof(block_t));
	DIE(!new_sfl->data, "Malloc failed while allocating data for new_sfl");

	// Set the data of the current node
	((block_t *)new_sfl->data)->address = (void *)(block_address);
	((block_t *)new_sfl->data)->size = block_size;

	// Find the list which matches the remaining size
	for (size_t j = 0; j < *lists_num; j++) {
		if (((block_t *)(*sfl_lists)[j].head->data)->size != block_size)
			continue;

		// Find the position of the new node in the segregated free list
		node_t *last_sfl = (*sfl_lists)[j].head;

		if (((block_t *)new_sfl->data)->address <
			((block_t *)last_sfl->data)->address) {
			new_sfl->next = (*sfl_lists)[j].head;
			new_sfl->prev = NULL;
			(*sfl_lists)[j].head->prev = new_sfl;
			(*sfl_lists)[j].head = new_sfl;
		} else {
			// Move to the appropriate position
			while (last_sfl->next &&
				   ((block_t *)last_sfl->next->data)->address <
					   ((block_t *)new_sfl->data)->address)
				last_sfl = last_sfl->next;

			// Add the current node to the segregated free list
			new_sfl->next = last_sfl->next;
			new_sfl->prev = last_sfl;
			last_sfl->next = new_sfl;
		}

		// Update the number of free blocks in the list
		(*sfl_lists)[j].size += 1;

		// Return if the list was found and updated
		return;
	}

	// If there is no list with the remaining size, add a new list

	// Update the number of lists
	*lists_num += 1;
	*sfl_lists = realloc(*sfl_lists, *lists_num * sizeof(list_t));
	DIE(!*sfl_lists, "Realloc failed while reallocating sfl_lists");

	// Find the index of the new list
	for (size_t j = 0; j < *lists_num; j++) {
		if (j == *lists_num - 1 ||
			((block_t *)(*sfl_lists)[j].head->data)->size > block_size) {
			// Move the lists to the right
			for (size_t k = *lists_num - 1; k > j; k--)
				(*sfl_lists)[k] = (*sfl_lists)[k - 1];

			// Add and initialise the new list
			(*sfl_lists)[j].size = 1;
			(*sfl_lists)[j].head = new_sfl;
			new_sfl->next = NULL;
			new_sfl->prev = NULL;

			return;
		}
	}
}

node_t *remove_ll_node(list_t *allocated_blocks, size_t block_address,
					   void *heap_data, size_t start_address)
{
	// Find the block with the given address
	for (node_t *current_ll = allocated_blocks->head; current_ll;
		 current_ll = current_ll->next) {
		if (block_address != (size_t)((block_t *)current_ll->data)->address -
								 (size_t)(char *)heap_data + start_address)
			continue;

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
		if (((block_t *)current_ll->data)->size == 0)
			allocated_blocks->head = NULL;

		// Return the current node
		return current_ll;
	}

	// Return NULL if the block was not found
	return NULL;
}
