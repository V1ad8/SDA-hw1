.PHONY: build clean run_sfl

build: sfl

sfl: src/main.c src/func/*.c
	gcc -g -Wall -Wextra -std=c99 src/main.c src/func/*.c -o sfl

run_sfl: sfl
	./sfl

clean:
	rm -f sfl

pack:
	zip -FSr 315CA_UngureanuVlad-Marin_Homework1.zip README.md Makefile src/