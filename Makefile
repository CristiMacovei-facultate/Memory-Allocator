build: main.c
	gcc -o sfl main.c

run: main.c
	make build
	./sfl

bd: main.c
	gcc -g -Wall -Wextra -pedantic -o sfl main.c 