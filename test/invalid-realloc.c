#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "jkmalloc.h"

int main(void)
{
	int foo = 42;
	int *ptr = jk_realloc(&foo, sizeof(int));
	printf("%d\n", *ptr);
}
