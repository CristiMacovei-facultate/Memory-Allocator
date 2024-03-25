#ifndef UTILS_H_GUARD
#define UTILS_H_GUARD

#include <string.h>
#include <stdlib.h>

char *strdup(char *str);

size_t atolx(char *str);

int starts_with(char *cmd, char *string);

#endif 