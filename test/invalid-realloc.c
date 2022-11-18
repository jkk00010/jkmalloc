#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
	int foo = 42;
	int *ptr = realloc(&foo, sizeof(int));
	printf("should not be reached: %d\n", *ptr);
}
