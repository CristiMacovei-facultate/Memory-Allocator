#include <string.h>
#include <stdlib.h>

char *strdup(char *str)
{
	char *ans = malloc(strlen(str) + 1);
	strcpy(ans, str);

	return ans;
}

size_t atolx(char *str)
{
#ifdef DEBUG_MODE
	printf("Passing %s into this shitty function\n", str);
#endif 

	size_t ans = 0;
	size_t n = strlen(str);
	for (size_t i = 0; i < n; ++i)
	{
		int digit = str[i] - '0';
		if ('a' <= tolower(str[i]) && tolower(str[i]) <= 'f')
			digit = 10 + tolower(str[i]) - 'a';
		ans = ans * 16 + digit;
	}
	return ans;
}

int starts_with(char *cmd, char *string)
{
	int n = strlen(string);
	for (int i = 0; i < n; ++i)
	{
		if (tolower(cmd[i]) != tolower(string[i]))
			return 0;
	}

	return 1;
}
