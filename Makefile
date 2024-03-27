.PHONY: build, clean

build:
	gcc -Wall -Wextra -pedantic -g -ggdb -O0 -march=native src/*.c -o sfl

run:
	./sfl

clean:
	rm -f sfl
