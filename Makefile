FILES = main.c arraylist.c dll.c

build: $(FILES)
	gcc -Wall -Wextra -pedantic -o sfl $(FILES)

run: $(FILES)
	make build
	./sfl

bd: $(FILES)
	gcc -g -Wall -Wextra -pedantic -o sfl $(FILES)