# SDA-homework_1: Segregated Free Lists

**Ungureanu Vlad-Marin 315CA**

## Task Description

### Functionalities:
The program provides the following functionalities:
* **INIT_HEAP**: Initializes the Segregated Free Lists data structure for a specified heap, with a given number of doubly linked lists, each holding blocks of free memory of the same size
* **MALLOC**: Allocates memory from the heap for a specified number of bytes
* **FREE**: Frees a specified previously allocated memory area
* **READ**: Reads a specified number of bytes from a specified address
* **WRITE**: Writes a character string to a specified memory address
* **DUMP_MEMORY**: Displays the current state of memory, including allocated and free blocks
* **DESTROY_HEAP**: Frees all allocated memory and terminates the program

### Commands
The described functionalities work by receiving the following inputs:
* **INIT_HEAP** <*start_address*> <*lists_num*> <*bytes_per_list*> <*reconstruct_type*>
* **MALLOC** <*block_size*>
* **FREE** <*block_address*>
* **READ** <*block_address*> <*read_size*>
* **WRITE** <*block_address*> <*text*> <*write_size*>
* **DUMP_MEMORY**
* **DESTROY_HEAP**

### Error Handling
The program handles various input or operational errors, including:
* **OUT_OF_MEMORY**: Error message displayed when there is not enough memory for allocation
* **INVALID_FREE**: Error message displayed when attempting to free a memory area that was not allocated or does not represent the beginning of a block
* **SEGMENTATION_FAULT**: Error message displayed when attempting to read from or write to an unallocated memory area or one that does not contain sufficient allocated memory

### Bonus Feature
The program also provides a bonus feature for reconstituting fragmented memory blocks upon deallocation.

## Usage
To use the program, follow these steps:
* Compile the program with the *make build* rule within the provided Makefile
```
vlad@Laptop:~SDA/homeworks/homework1$ make build
gcc -Wall -Wextra -std=c99 src/main.c src/func/*.c -o sfl
```
* Run the program
```
vlad@Laptop:~SDA/homeworks/homework1$ ./sfl
```
Or just use the *run_sfl* rule from the Makefile
```
vlad@Laptop:~SDA/homeworks/homework1$ make run_sfl 
gcc -Wall -Wextra -std=c99 src/main.c src/func/*.c -o sfl
./sfl
```

## Implementation Information
### Files:
The code is spread troughout six C source files to make reading them individually easier. The functions are divided as follows:
* **src/main.c**: main()
* **src/func/heap.c**: init_heap(), destroy_heap()
* **src/func/lists.c**: add_ll_node(), add_sfl_node(), remove_ll_node()
* **src/func/memory.c**: malloc_f(), defragmented(), free_f()
* **src/func/read-write.c**: read(), write(), dump_memory()
* **src/func/utils.c**: is_power(), read_text(), run()

These source files are supported by three header files:
* **src/header.h**: includes the definitions of all the functions
* **src/structs.h**: includes all the libraries, definitions and structs used by the program
* **src/utils.h** (borrowed from [the first lab skel](https://ocw.cs.pub.ro/courses/_media/sd-ca/laboratoare/lab01_recap_pc_skel.zip)): includes the definition of DIE()

### Structs & enums used
I used the following:
* bool
* two doubly linked lists: sfl_list_t & ll_list_t
* nodes for the lists: sfl_node_t & ll_node_t

The first list stores the size of the elements withing the list, while the second one stores it within each node.

The first one is a segregated free list, and the second one is the list of allocated blocks.

**Note**: The data inside the ll_node_t is a pointer to a real address from the memory. It only gets translated to the digital address when it is written(in the DUMP_MEMORY command) or when it is needed for searching in the lists and it is given in its digital form(in the FREE, READ and WRITE functions).

## Implementation
### run()
I moved all the functionalities of the program to the run() function, in order to leave the main() function (almost) empty. This function reads the commands from the *stdin* and calls other functions to do the job.

### INIT_HEAP
The init_heap() function is called. It manages everything for this command:
* declaration of the heap data
* declaration and initialization of the segregated free lists

All of the pointers form the segregated free lists point to a specific zone of the heap data, in order to ensure the continuity inside of the memory of the data stored in the allocated blocks.

### MALLOC
The malloc_f() function is called. It calls the add_ll_node() function to remove a block from the segregated free lists then add a part of it of the required size to the lists of allocated blocks. If there is no memory left for the malloc, then the function stops after the call of the add_ll_node() function and prints an error message. Afterwards, if the required size is smaller than the block size, the add_sfl_node() is called to add the rest of the block back to the segregated free lists.
```
Out of memory
```

### FREE
The free_f() function is called. It calls the remove_ll_node() function to check if the block is allocated and remove it from the list of allocated blocks. If the block is not allocated it prints an error message and stops itself. If the *reconstruct_type* is set to 1 (meaning the memory should be reconstructed when deallocated), the defragment() function is called as many times as it is necessary to reunite the block with all its compatible neighbors. It checks if the blocks to its right and left are in the segregated free lists, and modifies the size and address of the blocks correspondingly. It uses the is_power() function to be able to jump over the blocks that are already whole. Afterwards, the add_sfl_node() function is called to add the block in the segregated free lists.
```
Invalid free
```

### READ
The read() function is called. It searches for the provided block address in the list of allocated blocks and copies the data within the block(s) in a string. If before reading the full size, the function encounters a block that is not allocated, it prints an error message, dumps the memory by calling the dump_memory() function and stops the program. Otherwise, it prints the string that it read.
```
Segmentation fault (core dumped)
+++++DUMP+++++
:
:
-----DUMP-----
```

### WRITE
The write() function is called. It calls the read_text() function to read the string from the *stdin*. It then searches for the provided block address in the list of allocated blocks and copies the data within the string given from the input to the data of the block(s) byte by byte. If before writing the full size, the function encounters a block that is not allocated, it prints an error message, dumps the memory by calling the dump_memory() function and stops the program.
```
Segmentation fault (core dumped)
+++++DUMP+++++
:
:
-----DUMP-----
```

### DUMP_MEMORY
The dump_memory() function is called. It prints the statistics of the memory at the given time. In order to ensure the correct order of the blocks, every time a block is added to a list, it is added in its corresponding place in order to maintain the order.
```
+++++DUMP+++++
Total memory: <total_memory> bytes
Total allocated memory: <total_allocated_memory> bytes
Total free memory: <total_free_memory> bytes
Free blocks: <number_of_free_blocks>
Number of allocated blocks: <number_of_allocated_blocks>
Number of malloc calls: <number_of_malloc_calls>
Number of fragmentations: <number_of_fragmentations>
Number of free calls: <number_of_free_calls>
Blocks with 8 bytes - <number_of_free_blocks_in_list> free block(s) : <first_address> <second_address> ... <last_address>
Blocks with 16 bytes - <number_of_free_blocks_in_list> free block(s) : <first_address> <second_address> ... <last_address>
:
:
Blocks with <8 * 2 ^ n> bytes - <number_of_free_blocks_in_list> free block(s) : <first_address> <second_address> ... <last_address>
Allocated blocks : (<first_address> - <first_size>) (<second_address> - <second_size>) ... (<last_address> - <last_size>)
-----DUMP-----
```

### DESTROY_HEAP
The destroy_heap() function is called. It frees the memory used for:
* the segregated free lists
* the list of allocated blocks
* the data of the heap

## Personal Comments
### Do I believe I could have make a better implementation?
Yes. I think it could have been implemented in a easier way. Like, instead of using the ll_list_t, to use a simple linked list, or maybe use just one struct for both functionalities. Also, i don't particularly like the way the code looks. The debugging process was very overly complicated. As proof of this, i quit before debugging the last test. I hope it is easier to understand than to debug, and I hope this README made the process easier.

## What did I learn from this homework?
First of all, I learned how the memory works. Secondly, I learned how to use valgrind better and with more efficiency.

## Resources / Bibliography:
* [the first lab skel](https://ocw.cs.pub.ro/courses/_media/sd-ca/laboratoare/lab01_recap_pc_skel.zip) for the DIE() 'function'
* previous PCLP1 homeworks, from where I borrowed the Makefile