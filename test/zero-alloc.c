#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	char *ptr = malloc(0);
	long pagesize = sysconf(_SC_PAGESIZE);
	printf("ptr: %p, pagesize %ld\n", ptr, pagesize);
	ptr[0] = '\0';

	printf("should not be reached: %p\n", ptr);
}
