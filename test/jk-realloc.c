#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "jkmalloc.h"

int main(void)
{
	const char buf[] = "THIS IS A CONSTANT STRING";

	char *ptr = jk_realloc(NULL, sizeof(buf));
	memcpy(ptr, buf, sizeof(buf));
	printf("%p: %s\n", ptr, ptr);

	ptr = jk_realloc(ptr, sizeof(buf) * 2);
	memcpy(ptr + sizeof(buf) - 1, buf, sizeof(buf));
	printf("%p: %s\n", ptr, ptr);

	ptr = jk_realloc(ptr, sizeof(buf) * sysconf(_SC_PAGESIZE));
	memcpy(ptr + (2 * sizeof(buf)) - 2, buf, sizeof(buf));
	printf("%p: %s\n", ptr, ptr);
}
