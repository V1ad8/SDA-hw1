CC=gcc
CFLAGS=-Wall -Wextra -pedantic
DEBUG=-g -ggdb -O0 -march=native

ELF=segregated_free_lists

.PHONY: build, clean

build: $(ELF)

$(ELF): $(ELF).c
	$(CC) $(CFLAGS) $(DEBUG) $^ -o $@

run: $(ELF)
	./$<

clean:
	rm -f $(ELF)
