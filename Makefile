.PHONY: build, clean

build:
	gcc -Wall -Wextra -pedantic -g -ggdb -O0 -march=native src/sfl.c src/functions.c -o sfl

run:
	./sfl

clean:
	rm -f sfl
