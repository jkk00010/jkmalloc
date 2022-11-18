#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
	const char buf[] = "THIS IS A CONSTANT STRING";

	char *ptr = malloc(sizeof(buf));
	memcpy(ptr, buf, sizeof(buf));
	printf("%p: %s\n", ptr, ptr);

	free(ptr);
	printf("freed\n");

	printf("should not be reached: %p: %s\n", ptr, ptr);
}
