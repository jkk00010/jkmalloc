#include "mapalloc.h"

void *malloc(size_t n)
{
	return map_malloc(n);
}

void *calloc(size_t n, size_t e)
{
	return map_calloc(n, e);
}

void *realloc(void *p, size_t n)
{
	return map_realloc(p, n);
}

void free(void *p)
{
	map_free(p);
}
