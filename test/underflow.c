#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	char *ptr = malloc(1);
	long pagesize = sysconf(_SC_PAGESIZE);
	printf("ptr: %p, pagesize %ld\n", ptr, pagesize);
	ptr[-1] = '\0';
	printf("shouldn't get here\n");
}
