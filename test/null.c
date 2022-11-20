#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	char *ptr = malloc(1);	/* purely to load the signal handler */
	ptr = NULL;
	ptr[0] = '\0';
	puts("undetected: NULL pointer dereference");
}
