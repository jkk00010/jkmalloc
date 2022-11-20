#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>

#if defined __GNUC__ && !defined __clang__
#pragma GCC diagnostic ignored "-Wuse-after-free"
#endif

int main(void)
{
	char *ptr = malloc(1);
	free(ptr);
	*ptr = '\0';
	puts("undetected: use after free");
}
