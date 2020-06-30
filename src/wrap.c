#include "mapalloc.h"

void *malloc(size_t n)
{
	return MA_malloc(n);
}

void *calloc(size_t n, size_t e)
{
	return MA_calloc(n, e);
}

void *realloc(void *p, size_t n)
{
	return MA_realloc(p, n);
}

void free(void *p)
{
	MA_free(p);
}
