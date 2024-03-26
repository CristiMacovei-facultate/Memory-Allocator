// Nicolae-Cristian MACOVEI, Anul I, Grupa 312CAb

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"

void parse_init_args(char *cmd, size_t *ptr_start_addr, int *ptr_num_lists,
					 size_t *ptr_bytes_per_list, int *ptr_rec_type)
{
	char sep[] = " ";
	char *p = strtok(cmd, sep);

	int arg_idx = 0;
	while (arg_idx < 5 && p) {
		// start address
		if (arg_idx == 1) {
			size_t tmp_start_addr = atolx(p + 2);
			if (tmp_start_addr)
				*ptr_start_addr = tmp_start_addr;
		}
		// number of lists
		else if (arg_idx == 2) {
			int tmp_num_lists = atoi(p);
			if (tmp_num_lists)
				*ptr_num_lists = tmp_num_lists;
		}
		// bytes per list
		else if (arg_idx == 3) {
			size_t tmp_bpl = atol(p);
			if (tmp_bpl)
				*ptr_bytes_per_list = tmp_bpl;
		}
		// type of reconstruction
		else if (arg_idx == 4) {
			size_t tmp_type = atol(p);
			if (tmp_type)
				*ptr_rec_type = tmp_type;
		}

		++arg_idx;
		p = strtok(NULL, sep);
	}
}

void parse_malloc_args(char *cmd, size_t *ptr_num_bytes)
{
	char sep[] = " ";
	char *p = strtok(cmd, sep);

	int arg_index = 0;
	while (arg_index < 2 && p) {
	#ifdef DEBUG_MODE
		printf("Malloc arg [%d]: '%s'\n", arg_index, p);
	#endif

		// start address
		if (arg_index == 1) {
			size_t tmp_requested = atol(p);
			if (tmp_requested)
				*ptr_num_bytes = tmp_requested;
		}

		++arg_index;
		p = strtok(NULL, sep);
	}
}

void parse_read_args(char *cmd, size_t *ptr_addr, size_t *ptr_num_bytes)
{
	char sep[] = " ";
	char *p = strtok(cmd, sep);

	int arg_index = 0;
	while (arg_index < 4 && p) {
		// printf("Arg %d: '%s'\n", i, p);

		// start address
		if (arg_index == 1) {
			size_t tmp_addr = atolx(p + 2);
			if (tmp_addr)
				*ptr_addr = tmp_addr;
		} else if (arg_index == 2) {
			size_t tmp_num_bytes = atoll(p);
			if (tmp_num_bytes)
				*ptr_num_bytes = tmp_num_bytes;
		}

		++arg_index;
		p = strtok(NULL, sep);
	}
}

void parse_write_args(char *cmd, size_t *ptr_addr, char **ptr_data,
					  size_t *ptr_num_bytes)
{
	strcat(cmd, " ");
	size_t n = strlen(cmd);

	int arg_index = 0;
	size_t prev_space = -1;
	int num_quotes = 0;
	for (size_t i = 0; i < n; ++i) {
		if (cmd[i] == '"')
			num_quotes = 1 - num_quotes;

		if (cmd[i] == ' ' && num_quotes == 0) {
			if (prev_space != (i - 1)) {
				cmd[i] = '\0';

				char *arg = (cmd + prev_space + 1);

				// start address
				if (arg_index == 1) {
					size_t tmp_addr = atolx(arg + 2);
					if (tmp_addr)
						*ptr_addr = tmp_addr;
				} else if (arg_index == 2) {
					if (arg[0] == '"') {
						arg[strlen(arg) - 1] = '\0';
						*ptr_data = strdup(arg + 1);
						arg[strlen(arg)] = ' ';
					} else {
						*ptr_data = strdup(arg);
					}
				} else if (arg_index == 3) {
					size_t tmp_num_bytes = atoll(arg);
					if (tmp_num_bytes)
						*ptr_num_bytes = tmp_num_bytes;
				}

				cmd[i] = ' ';
				++arg_index;
			}

			prev_space = i;
		}
	}
}
