#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	char *ptr = malloc(0);
	ptr[0] = '\0';
	puts("undetected: use of 0-byte allocation");
}
