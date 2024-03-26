FILES = main.c arraylist.c dll.c utils.c cmd_utils.c commands/print.c commands/malloc.c commands/read.c commands/write.c commands/destroy.c commands/init.c commands/free.c
CFLAGS = -Wall -Wextra -pedantic -std=c99

build: $(FILES)
	gcc $(CFLAGS) -o sfl $(FILES)

run_sfl: $(FILES)
	make build
	./sfl

buildd: $(FILES)
	gcc -g -D DEBUG_MODE $(CFLAGS) -o sfl $(FILES)

rund: $(FILES)
	make buildd
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./sfl

clean: 
	rm ./sfl