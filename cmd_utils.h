#ifndef CMD_UTILS_H_GUARD
#define CMD_UTILS_H_GUARD

#include <stdlib.h>

void parse_init_args(char *cmd, size_t *ptr_start_addr, int *ptr_num_lists,
					 size_t *ptr_bytes_per_list, int *ptr_rec_type);

void parse_malloc_args(char *cmd, size_t *ptr_num_bytes);

void parse_read_args(char *cmd, size_t *ptr_addr, size_t *ptr_num_bytes);

void parse_write_args(char *cmd, size_t *ptr_addr, char **ptr_data,
					  size_t *ptr_num_bytes);

#endif
