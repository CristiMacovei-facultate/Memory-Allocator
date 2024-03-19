FILES = main.c arraylist.c

build: $(FILES)
	gcc -o sfl $(FILES)

run: $(FILES)
	make build
	./sfl

bd: $(FILES)
	gcc -g -Wall -Wextra -pedantic -o sfl $(FILES)