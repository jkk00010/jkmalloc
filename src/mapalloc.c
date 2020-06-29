#define _POSIX_C_SOURCE 200809L
#include <sys/mman.h>
#include <string.h>

#include "mapalloc.h"

void *map_calloc(size_t nelem, size_t elsize)
{
	size_t n = nelem * elsize;
	if (n < nelem || n < elsize) {
		/* overflow */
		return NULL;
	}
	void *ptr = map_malloc(n);
	memset(ptr, 0, n);
	return ptr;
}

void *map_malloc(size_t n)
{
}

void *map_realloc(void *ptr, size_t n)
{
	if (ptr == NULL) {
		return map_malloc(n);
	}
}

void map_free(void *ptr)
{
	if (ptr == NULL) {
		return;
	}
}
