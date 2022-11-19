#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	char *ptr = malloc(1);
	long pagesize = sysconf(_SC_PAGESIZE);
	printf("ptr: %p, pagesize %ld\n", (void*)ptr, pagesize);
	for (int i = 0; i <= pagesize; i++) {
		ptr[i] = '\0';
	}
	printf("should not be reached: %p\n", (void*)ptr);
}
