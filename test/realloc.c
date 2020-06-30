#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "mapalloc.h"

int main(void)
{
	const char buf[] = "THIS IS A CONSTANT STRING";

	char *ptr = map_realloc(NULL, sizeof(buf));
	memcpy(ptr, buf, sizeof(buf));
	printf("%p: %s\n", ptr, ptr);

	ptr = map_realloc(ptr, sizeof(buf) * 2);
	memcpy(ptr + sizeof(buf), buf, sizeof(buf));
	printf("%p: %s\n", ptr, ptr);
}
