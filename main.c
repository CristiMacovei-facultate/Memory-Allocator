#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "cmd_utils.h"
#include "utils.h"
#include "dll.h"
#include "arraylist.h"
#include "structs.h"

#include "commands/init.h"
#include "commands/print.h"
#include "commands/malloc.h"
#include "commands/read.h"
#include "commands/write.h"
#include "commands/free.h"
#include "commands/destroy.h"

#define CMD_LINE 600

int main(void)
{
	char cmd[CMD_LINE];

	sfl_t *list = NULL;

	while (!feof(stdin)) {
		fgets(cmd, CMD_LINE, stdin);

		if (cmd[strlen(cmd) - 1] == '\n')
			cmd[strlen(cmd) - 1] = '\0';

		if (starts_with(cmd, "INIT_HEAP"))
			handle_init(cmd, &list);
		#ifdef DEBUG_MODE
		else if (starts_with(cmd, "PRINT"))
			handle_print(list);
		#endif
		else if (starts_with(cmd, "DUMP_MEMORY"))
			handle_print(list);
		else if (starts_with(cmd, "MALLOC"))
			handle_malloc(cmd, list);
		else if (starts_with(cmd, "READ")) {
			handle_read(cmd, &list);

			if (!list)
				return 0;
		} else if (starts_with(cmd, "WRITE")) {
			handle_write(cmd, &list);

			if (!list)
				return 0;
		} else if (starts_with(cmd, "FREE"))
			handle_free(cmd, list);
		else if (starts_with(cmd, "DESTROY_HEAP")) {
			handle_destroy(&list);
			return 0;
		}
	}

	return 0;
}
