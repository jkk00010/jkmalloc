#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "jkmalloc.h"

int main(void)
{
	const char buf[] = "THIS IS A CONSTANT STRING";

	char *ptr = jk_malloc(sizeof(buf));
	memcpy(ptr, buf, sizeof(buf));
	printf("%p: %s\n", ptr, ptr);

	jk_free(ptr);
	printf("freed\n");
	printf("%p: %s\n", ptr, ptr);
}
