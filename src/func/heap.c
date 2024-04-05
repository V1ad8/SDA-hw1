#include "../header.h"

list_t *init_heap(size_t heap_start, size_t lists_num, size_t bytes_per_list,
				  void **heap_data)
{
	// Allocate memory for the heap
	*heap_data = malloc(lists_num * bytes_per_list);
	DIE(!*heap_data, "Malloc failed while allocating heap_data");

	// Set the starting address of the heap
	heap_start = (size_t)*heap_data;

	// Allocate memory for the segregated free lists
	list_t *sfl_lists = malloc(sizeof(list_t) * lists_num);
	DIE(!sfl_lists, "Malloc failed while allocating sfl_lists");

	// Initialize each segregated free list
	for (size_t i = 0; i < lists_num; i++) {
		// Calculate the element size and size of the current list
		size_t element_size = 8 * (1 << i);
		sfl_lists[i].size = bytes_per_list / element_size;

		// Create the head node for the current list
		node_t *previous = malloc(sizeof(node_t));
		DIE(!previous, "Malloc failed while allocating node for sfl_lists[i]");
		previous->data = malloc(sizeof(block_t));
		DIE(!previous->data,
			"Malloc failed while allocating data for sfl_lists[i]");

		((block_t *)(previous->data))->address =
			(void *)(heap_start + i * bytes_per_list);
		((block_t *)(previous->data))->size = element_size;

		sfl_lists[i].head = previous;
		previous->prev = NULL;

		// Create the remaining nodes for the current list
		for (size_t j = 1; j < sfl_lists[i].size; j++) {
			node_t *current = malloc(sizeof(node_t));
			DIE(!current,
				"Malloc failed while allocating node for sfl_lists[i]");

			// Set the data of the current node
			current->data = malloc(sizeof(block_t));
			DIE(!current->data,
				"Malloc failed while allocating data for sfl_lists[i]");
			((block_t *)(current->data))->address =
				(void *)(heap_start + i * bytes_per_list + j * element_size);
			((block_t *)(current->data))->size = element_size;

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

void destroy_heap(list_t *sfl_lists, size_t lists_num, void *heap_data,
				  list_t allocated_blocks)
{
	// Free the memory of the segregated free lists's nodes
	for (size_t i = 0; i < lists_num; i++) {
		node_t *current = sfl_lists[i].head;
		while (current) {
			node_t *next = current->next;
			free(current->data);
			free(current);
			current = next;
		}
	}

	// Free the memory of the allocated blocks nodes
	node_t *current = allocated_blocks.head;
	while (current) {
		node_t *next = current->next;
		free(current->data);
		free(current);
		current = next;
	}

	// Free the memory of the segregated free lists
	free(sfl_lists);

	// Free the memory of the heap
	free(heap_data);
}
