#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined __GNUC__ && !defined __clang__
#pragma GCC diagnostic ignored "-Wuse-after-free"
#endif

int main(void)
{
	const char buf[] = "THIS IS A CONSTANT STRING";
	char *ptr = malloc(sizeof(buf));
	memcpy(ptr, buf, sizeof(buf));
	free(ptr);
	free(ptr);
	puts("undetected: double free");
}
