build: main.c
	gcc -o sfl main.c

run: main.c
	make build
	./sfl