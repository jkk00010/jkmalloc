#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#endif

int main(void)
{
	int foo = 42;
	free(&foo);
	puts("undetected: free() on automatic address");
}
